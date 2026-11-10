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

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < JUMLAH; i++) {
    pinMode(lampu[i].pin, OUTPUT);
    digitalWrite(lampu[i].pin, LOW);
  }
  tls.setInsecure();   // tanpa cek sertifikat: koneksi tetap terenkripsi
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.printf("\nWiFi tersambung  IP: %s\n", WiFi.localIP().toString().c_str());
}

void poll(int i) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi putus");
    WiFi.reconnect();
    return;
  }

  HTTPClient http;
  http.setTimeout(5000);
  if (!http.begin(tls, URL)) {
    Serial.printf("%s: begin gagal\n", lampu[i].nama);
    return;
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
}

void loop() {
  for (int i = 0; i < JUMLAH; i++) poll(i);
  delay(1000);   // satu putaran per detik
}
