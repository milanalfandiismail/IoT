#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* SSID     = "TM_2.4G";
const char* PASSWORD = "";
const char* URL_LED  = "https://iot.milannn.my.id/api/device/state";
const char* URL_SUHU = "https://iot.milannn.my.id/api/device/temperature";

#define DHT_TYPE DHT22
#define BACA_MS  1000

struct Lampu {
  const char* nama;
  const char* token;
  uint8_t pin;
  bool nyala;
};

Lampu lampu[] = {
  {"led-1", "EIFwyngGMzPB8xunORbenH3K-lesant8T9SpN_90FJg", 2, false},
  {"led-2", "s5wlaSXYwRehVuyhYNlwpPHBJob5-uic2cLuJ97XRHw", 4, false},
};
const int JUMLAH = sizeof(lampu) / sizeof(lampu[0]);

struct Suhu {
  const char* nama;
  const char* token;
  uint8_t pin;
  DHT* dht;
  unsigned long terakhir;
};

Suhu suhu[] = {
  {"dht-1", "SRPS32h2CAiyns8BoSEi2ApvY8FV1eI2WEuqWlbxT9Y", 5, nullptr, 0},
};
const int JUMLAH_SUHU = sizeof(suhu) / sizeof(suhu[0]);

WiFiClientSecure tls;
WiFiClient biasa;
HTTPClient http;   // dipakai ulang antar-request (keep-alive)

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
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE,
              IPAddress(9, 9, 9, 9), IPAddress(1, 1, 1, 1));
  WiFi.begin(SSID, PASSWORD);
  unsigned long mulai = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - mulai < 20000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi gagal tersambung");
    return;
  }
  Serial.printf("\nWiFi tersambung  IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("SSID: %s  GW: %s  DNS: %s\n", WiFi.SSID().c_str(),
                WiFi.gatewayIP().toString().c_str(),
                WiFi.dnsIP().toString().c_str());
}

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

bool mulaiHttp(HTTPClient& http, const char* url) {
  return String(url).startsWith("https://") ? http.begin(tls, url) : http.begin(biasa, url);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < JUMLAH; i++) {
    pinMode(lampu[i].pin, OUTPUT);
    digitalWrite(lampu[i].pin, LOW);
  }
  for (int i = 0; i < JUMLAH_SUHU; i++) {
    suhu[i].dht = new DHT(suhu[i].pin, DHT_TYPE);
    suhu[i].dht->begin();
  }
  tls.setInsecure();
  http.setReuse(true);   // pertahankan koneksi, hindari handshake tiap poll
  http.setTimeout(3000);
  sambungWiFi();
  for (int i = 0; i < 5 && !tungguDNS(); i++) sambungWiFi();
}

int pollSekali(int i) {
  unsigned long t0 = millis();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi putus");
    return -1;
  }

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
  Serial.printf("%s: %lu ms\n", lampu[i].nama, millis() - t0);
  return code;
}

// Koneksi keep-alive bisa sudah mati di sisi server atau WiFi, sedangkan
// HTTPClient masih mengira hidup. Kalau gagal, tutup lalu coba sekali lagi
// dengan koneksi baru.
int poll(int i) {
  int code = pollSekali(i);
  if (code < 0) {
    Serial.printf("%s: ulang dengan koneksi baru\n", lampu[i].nama);
    http.end();
    code = pollSekali(i);
  }
  return code;
}

void kirimSuhu(int i) {
  if (millis() - suhu[i].terakhir < BACA_MS) return;
  suhu[i].terakhir = millis();

  float celsius = suhu[i].dht->readTemperature();
  float humidity = suhu[i].dht->readHumidity();
  if (isnan(celsius) || isnan(humidity)) {
    Serial.printf("[%s] gagal baca, periksa pull-up 10k di pin data\n", suhu[i].nama);
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[%s] WiFi putus, data dibuang\n", suhu[i].nama);
    return;
  }

  if (!mulaiHttp(http, URL_SUHU)) {
    Serial.printf("[%s] begin gagal\n", suhu[i].nama);
    return;
  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + suhu[i].token);
  String body = String("{\"celsius\":") + String(celsius, 1) +
                String(",\"humidity\":") + String(humidity, 1) + "}";
  int code = http.POST(body);
  Serial.printf("[%s] POST %d body: %s\n", suhu[i].nama, code, body.c_str());
  http.end();
}

void loop() {
  bool gagal = false;

  for (int i = 0; i < JUMLAH; i++) {
    if (poll(i) < 0) gagal = true;
  }

  for (int i = 0; i < JUMLAH_SUHU; i++) {
    kirimSuhu(i);
  }

  // Sambung ulang WiFi supaya DNS di-resolve lagi dan dapat IP server yang baru
  if (gagal) {
    Serial.println("jaringan bermasalah, sambung ulang WiFi");
    sambungWiFi();
    for (int i = 0; i < 5 && !tungguDNS(); i++) sambungWiFi();
  }

  delay(500);
}