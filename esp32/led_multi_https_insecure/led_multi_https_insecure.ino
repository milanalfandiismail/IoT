// Versi ringkas untuk demo: HTTPS tanpa verifikasi sertifikat (setInsecure).
// Satu sketch untuk semua perangkat: LED (polling) + DHT22 (kirim suhu/kelembapan).
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* SSID       = "SSID_WIFI";
const char* PASSWORD   = "PASSWORD_WIFI";
const char* URL_LED    = "https://DOMAIN-ANDA/api/device/state";
const char* URL_SUHU   = "https://DOMAIN-ANDA/api/device/temperature";
const char* TOKEN_SUHU = "ganti-token-suhu";

#define DHT_PIN  4
#define DHT_TYPE DHT22
#define BACA_MS  5000     // DHT22 minimal 2 detik sekali

struct Lampu {
  const char* nama;   // label di Serial Monitor
  const char* token;  // token device dari kartu dashboard
  uint8_t pin;
  bool nyala;
};

// Satu baris = satu LED. Pin aman: 2, 4, 5, 12-19, 21-23, 25-27, 32, 33.
Lampu lampu[] = {
  {"led-1", "ganti-token-led-1", 2, false},
  {"led-2", "ganti-token-led-2", 4, false},
};
const int JUMLAH = sizeof(lampu) / sizeof(lampu[0]);

WiFiClientSecure tls;
WiFiClient biasa;
DHT dht(DHT_PIN, DHT_TYPE);
unsigned long terakhirSuhu = 0;

// Host polos dari URL, misal "iot.contoh.id".
String hostUrl(const char* url) {
  String s(url);
  int a = s.indexOf("://");
  a = (a < 0) ? 0 : a + 3;
  int b = s.indexOf('/', a);
  if (b < 0) b = s.length();
  return s.substring(a, b);
}

void sambungWiFi() {
  WiFi.disconnect();
  delay(500);
  // IP tetap dari DHCP, tapi DNS dipaksa ke 9.9.9.9 (DNS router kadang bohong,
  // dan Quad9 mengembalikan IP Cloudflare yang tidak diblokir ISP lebih dulu).
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE,
              IPAddress(9, 9, 9, 9), IPAddress(1, 1, 1, 1));
  WiFi.begin(SSID, PASSWORD);
  unsigned long mulai = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - mulai < 20000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {   // timeout: jangan lanjut dengan IP 0.0.0.0
    Serial.println("\nWiFi gagal tersambung");
    return;
  }
  Serial.printf("\nWiFi tersambung  IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("SSID: %s  GW: %s  DNS: %s\n", WiFi.SSID().c_str(),
                WiFi.gatewayIP().toString().c_str(),
                WiFi.dnsIP().toString().c_str());
}

// Hotspot HP bisa memberi IP sebelum NAT/DNS-nya siap; itu penyebab
// "poll gagal -1" di boot pertama yang dulu baru sembuh setelah restart.
bool tungguDNS() {
  IPAddress ip;
  for (int i = 0; i < 20; i++) {
    if (WiFi.hostByName(hostUrl(URL_LED).c_str(), ip)) {
      Serial.printf("DNS ok: %s\n", ip.toString().c_str());
      return true;
    }
    Serial.print("d");
    delay(1000);
  }
  return false;
}

// https:// pakai TLS tanpa verifikasi, http:// pakai client biasa.
bool mulaiHttp(HTTPClient& http, const char* url) {
  return String(url).startsWith("https://") ? http.begin(tls, url) : http.begin(biasa, url);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < JUMLAH; i++) {
    pinMode(lampu[i].pin, OUTPUT);
    digitalWrite(lampu[i].pin, LOW);
  }
  dht.begin();
  tls.setInsecure();   // tanpa cek sertifikat: koneksi tetap terenkripsi
  sambungWiFi();
  for (int i = 0; i < 5 && !tungguDNS(); i++) sambungWiFi();
}

int poll(int i) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi putus");
    return -1;
  }

  HTTPClient http;
  http.setTimeout(3000);
  if (!mulaiHttp(http, URL_LED)) {
    Serial.printf("%s: begin gagal\n", lampu[i].nama);
    return -2;
  }
  http.addHeader("Authorization", String("Bearer ") + lampu[i].token);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    bool on = body.indexOf("true") >= 0;
    if (on != lampu[i].nyala) {
      lampu[i].nyala = on;
      Serial.printf("%s (GPIO%d) %s\n", lampu[i].nama, lampu[i].pin, on ? "ON" : "OFF");
    }
    digitalWrite(lampu[i].pin, on ? HIGH : LOW);
    Serial.printf("%s body: %s\n", lampu[i].nama, body.c_str());
  } else {
    Serial.printf("%s: poll gagal: %d (%s)\n", lampu[i].nama, code,
                  HTTPClient::errorToString(code).c_str());
  }
  http.end();
  return code;
}

void kirimSuhu() {
  if (millis() - terakhirSuhu < BACA_MS) return;
  terakhirSuhu = millis();

  float celsius = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(celsius) || isnan(humidity)) {
    Serial.println("[DHT] gagal baca, periksa pull-up 10k di pin data");
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Suhu] WiFi putus, data dibuang");
    return;
  }

  HTTPClient http;
  http.setTimeout(3000);
  if (!mulaiHttp(http, URL_SUHU)) {
    Serial.println("[Suhu] begin gagal");
    return;
  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + TOKEN_SUHU);
  String body = String("{\"celsius\":") + String(celsius, 1) +
                String(",\"humidity\":") + String(humidity, 1) + "}";
  int code = http.POST(body);
  Serial.printf("[Suhu] POST %d body: %s\n", code, body.c_str());
  http.end();
}

void loop() {
  for (int i = 0; i < JUMLAH; i++) {
    int code = poll(i);
    // Kode < 0 = masalah jaringan/sambungan → reset. 401/500 berarti server
    // terjangkau, jadi cukup dicoba lagi putaran berikutnya.
    if (code < 0) {
      Serial.println("jaringan bermasalah, reset board");
      delay(1000);
      ESP.restart();
    }
  }

  kirimSuhu();
  delay(1000);   // satu putaran per detik
}
