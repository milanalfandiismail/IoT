/*
   ESP32 + LED + DHT22 lewat Blynk
   Pin: LED GPIO2 (built-in), DHT22 data di GPIO4 (D4)
   Virtual pin: V2 = kontrol LED, V3 = suhu, V4 = kelembapan

   Catatan stabilitas:
   - DHT22 butuh jeda minimal 2 detik antar pembacaan.
   - Modem sleep WiFi dimatikan agar tidak memicu interrupt saat
     library DHT menahan interrupt untuk membaca sensor.
*/

#define BLYNK_TEMPLATE_ID   "ISI_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ISI_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "ISI_TOKEN_BLYNK"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char ssid[] = "ISI_SSID";
char pass[] = "ISI_PASSWORD";

#define DHTPIN  4
#define DHTTYPE DHT22          // DHT22 / AM2302
#define BACA_MS 2000           // DHT22 minimal 2 detik sekali

const int ledPin = 2;

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledState = false;
bool wifiSiap = false;
unsigned long terakhirBlynk = 0;

// Tombol LED di aplikasi Blynk
BLYNK_WRITE(V2) {
  ledState = param.asInt();
  digitalWrite(ledPin, ledState ? HIGH : LOW);
  Serial.printf("[Blynk] LED %s\n", ledState ? "ON" : "OFF");
}

// Setiap kali tersambung, ambil status LED terakhir dari server
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V2);
}

void bacaSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    // Gagal baca biasanya pull-up kurang atau kabel data terlalu panjang
    Serial.println("[DHT] gagal baca, coba lagi 2 detik lagi");
    return;
  }

  Serial.printf("[DHT] %.1f C, %.1f %%RH\n", t, h);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V3, t);
    Blynk.virtualWrite(V4, h);
  }
}

void cekKoneksi() {
  if (millis() - terakhirBlynk < 10000) return;
  terakhirBlynk = millis();

  if (WiFi.status() != WL_CONNECTED) {
    wifiSiap = false;
    Serial.println("[WiFi] terputus, menyambung ulang");
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    return;
  }

  if (!wifiSiap) {
    wifiSiap = true;
    Serial.printf("[WiFi] IP %s, RSSI %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
  }

  if (!Blynk.connected()) {
    Serial.println("[Blynk] offline, mencoba sambung ulang");
    Blynk.connect(3000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  dht.begin();

  Serial.println();
  Serial.println("--------------------------------------------------");
  Serial.println("[Sistem] mulai");

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);        // cegah interrupt WiFi saat DHT membaca
  WiFi.begin(ssid, pass);

  Serial.printf("[WiFi] menyambung ke %s\n", ssid);
  unsigned long mulai = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - mulai < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiSiap = true;
    Serial.printf("[WiFi] tersambung, IP %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WiFi] belum tersambung, akan dicoba ulang");
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(3000);

  timer.setInterval(10000L, cekKoneksi);
  timer.setInterval(BACA_MS, bacaSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
