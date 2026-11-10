// Versi ringkas untuk demo: HTTPS tanpa verifikasi sertifikat (setInsecure).
// Cocok untuk tugas/demo di jaringan yang tidak bisa diatur. Jangan dipakai
// untuk produksi karena server bisa ditiru tanpa terdeteksi.
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* SSID     = "SSID_WIFI";
const char* PASSWORD = "PASSWORD_WIFI";
const char* URL      = "https://DOMAIN-ANDA/api/device/state";

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

// Host polos dari URL, misal "iot.contoh.id".
String hostUrl() {
  int a = String(URL).indexOf("://");
  a = (a < 0) ? 0 : a + 3;
  int b = String(URL).indexOf('/', a);
  if (b < 0) b = String(URL).length();
  return String(URL).substring(a, b);
}

void sambungWiFi() {
  WiFi.disconnect();
  delay(500);
  WiFi.begin(SSID, PASSWORD);
  unsigned long mulai = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - mulai < 20000) {
    delay(300);
    Serial.print(".");
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
    if (WiFi.hostByName(hostUrl().c_str(), ip)) {
      Serial.printf("DNS ok: %s\n", ip.toString().c_str());
      return true;
    }
    Serial.print("d");
    delay(1000);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < JUMLAH; i++) {
    pinMode(lampu[i].pin, OUTPUT);
    digitalWrite(lampu[i].pin, LOW);
  }
  tls.setInsecure();   // tanpa cek sertifikat: koneksi tetap terenkripsi
  sambungWiFi();
  for (int i = 0; i < 5 && !tungguDNS(); i++) sambungWiFi();
}

void uji(const char* host, int port) {
  WiFiClient c;
  unsigned long t = millis();
  bool ok = c.connect(host, port, 3000);
  Serial.printf("uji %s:%d -> %s (%lu ms)\n", host, port, ok ? "OK" : "GAGAL", millis() - t);
  c.stop();
}

// Dipanggil saat poll gagal: bedakan DNS/NAT belum siap vs diblokir portal.
void diagnosa() {
  Serial.printf("GW: %s\n", WiFi.gatewayIP().toString().c_str());
  uji(WiFi.gatewayIP().toString().c_str(), 80);
  uji("1.1.1.1", 443);
  uji("8.8.8.8", 443);
  uji("www.google.com", 443);
  uji("104.21.44.235", 443);   // IP Cloudflare hasil DNS di jaringan ini
  uji(hostUrl().c_str(), 80);
  uji(hostUrl().c_str(), 443);
}

bool poll(int i) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi putus");
    return false;
  }

  HTTPClient http;
  http.setTimeout(3000);
  // https:// pakai TLS tanpa verifikasi, http:// pakai client biasa.
  bool https = String(URL).startsWith("https://");
  if (!(https ? http.begin(tls, URL) : http.begin(biasa, URL))) {
    Serial.printf("%s: begin gagal\n", lampu[i].nama);
    return false;
  }
  http.addHeader("Authorization", String("Bearer ") + lampu[i].token);
  int code = http.GET();
  bool ok = (code == 200);
  if (ok) {
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
  return ok;
}

int gagal = 0;

void loop() {
  bool semua = true;
  for (int i = 0; i < JUMLAH; i++) semua = poll(i) && semua;

  gagal = semua ? 0 : gagal + 1;
  if (gagal == 2) diagnosa();
  if (gagal >= 2) {
    Serial.println("2x gagal, sambung ulang WiFi");
    sambungWiFi();
    tungguDNS();
    gagal = 0;
    delay(1000);
  }
  delay(1000);   // satu putaran per detik
}
