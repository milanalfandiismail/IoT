#include <WiFi.h>
#include <HTTPClient.h>

const char* SSID     = "SSID_WIFI";
const char* PASSWORD = "PASSWORD_WIFI";
const char* URL      = "http://10.10.10.25:5000/api/device/state";

struct Lampu {
  const char* nama;   // label di Serial Monitor
  const char* token;  // token device dari kartu dashboard
  uint8_t pin;
  bool nyala;
  unsigned long terakhir;
};

// Satu baris = satu LED di dashboard. Tambah LED dengan menambah baris.
// Pin aman: 2, 4, 5, 12-19, 21-23, 25-27, 32, 33. Hindari 6-11 (flash),
// 34-39 (input only), dan 0/2/12/15 saat boot bisa mengganggu upload.
Lampu lampu[] = {
  {"led-1", "ganti-token-led-1", 2, false, 0},
  {"led-2", "ganti-token-led-2", 4, false, 0},
};
const int JUMLAH = sizeof(lampu) / sizeof(lampu[0]);

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < JUMLAH; i++) {
    pinMode(lampu[i].pin, OUTPUT);
    digitalWrite(lampu[i].pin, LOW);
  }
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi tersambung");
}

void poll(int i) {
  HTTPClient http;
  http.setTimeout(2000);
  http.begin(URL);
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
    Serial.printf("%s: poll gagal: %d\n", lampu[i].nama, code);
  }
  http.end();
}

void loop() {
  for (int i = 0; i < JUMLAH; i++) {
    if (millis() - lampu[i].terakhir < 1000) continue;
    lampu[i].terakhir = millis();
    poll(i);
  }
  delay(10);
}
