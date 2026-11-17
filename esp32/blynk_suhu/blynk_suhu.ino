/*
   ESP32 + LED + DHT11 lewat Blynk
   Pin: LED GPIO2 (built-in), DHT11 data di GPIO4 (D4)
   Virtual pin: V2 = kontrol LED, V3 = suhu
*/

#define BLYNK_TEMPLATE_ID   "ISI_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ISI_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "ISI_TOKEN_BLYNK"   // tempel Auth Token dari Blynk Console

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char ssid[] = "TM_2.4G";
char pass[] = "";

#define DHTPIN 4
#define DHTTYPE DHT11      // ganti DHT22 kalau sensornya DHT22
DHT dht(DHTPIN, DHTTYPE);

const int ledPin = 2;
bool ledState = false;
unsigned long terakhirCekBlynk = 0;

BlynkTimer timer;

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

void sendSensorData() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("[DHT] gagal baca, periksa kabel data di GPIO4");
  } else {
    Blynk.virtualWrite(V3, t);
    Serial.printf("[DHT] suhu %.1f C\n", t);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] IP %s | RSSI %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    Serial.println("[WiFi] terputus, menyambung ulang");
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
  }
  Serial.println("--------------------------------------------------");
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
  Serial.printf("[WiFi] MAC %s\n", WiFi.macAddress().c_str());
  Serial.printf("[WiFi] menyambung ke %s\n", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  unsigned long mulai = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - mulai < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] tersambung, IP %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] gagal 10 detik, MAC belum di-bypass di MikroTik?");
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(5000);

  timer.setInterval(5000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();

  // Sambung ulang Blynk tiap 10 detik kalau putus
  if (!Blynk.connected() && millis() - terakhirCekBlynk > 10000) {
    terakhirCekBlynk = millis();
    Serial.println("[Blynk] offline, mencoba sambung ulang");
    Blynk.connect(3000);
  }
}
