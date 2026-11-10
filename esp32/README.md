# Firmware ESP32

Dua sketch, pilih sesuai device yang dibuat di dashboard:

- `led_poll/led_poll.ino` — polling `GET /api/device/state` tiap 1 detik, LED di
  GPIO2 mengikuti respons `{"on": true|false}`. Pakai token device bertipe **LED**.
- `dht_post/dht_post.ino` — baca DHT11/DHT22 tiap 5 detik, kirim
  `POST /api/device/temperature`. Pakai token device bertipe **Suhu**.

## Yang harus diubah sebelum upload

1. `SSID` dan `PASSWORD` — Wi-Fi yang sama dengan laptop server.
2. `URL` — ganti `10.10.10.25` dengan IP laptop hasil `ipconfig` (IPv4 Address).
   Pakai IP LAN, bukan `127.0.0.1`.
3. `TOKEN` — token device dari kartu **Lihat token** di dashboard.

Untuk `dht_post`, sesuaikan juga `DHT_PIN` (default GPIO4) dan `DHT_TYPE`
(`DHT22` atau `DHT11`).

## Upload

1. Board Manager: pasang paket **esp32** (Espressif Systems).
2. Library Manager: pasang **DHT sensor library** (Adafruit) + **Adafruit
   Unified Sensor** — hanya untuk `dht_post`.
3. Buka sketch, pilih board **ESP32 Dev Module**, port COM yang muncul.
4. Upload, lalu buka Serial Monitor **115200**.

## Yang seharusnya terlihat

`led_poll`:

- Titik-titik saat menyambung Wi-Fi, lalu `WiFi tersambung`.
- LED mengikuti tombol di dashboard dalam maksimal ~1 detik.
- Server dimatikan → `poll gagal: -1`, LED tetap di state terakhir.
- Server dihidupkan lagi → LED menyamakan diri dalam 1 detik.

`dht_post`:

- `POST 200 body: {"celsius":27.4}` tiap 5 detik.
- Angka suhu di kartu dashboard ikut berubah.
- `POST 400` → token dipakai di device bertipe LED, atau suhu di luar `-50..80`.
- `DHT gagal dibaca` → kabel data/sensor bermasalah.

## Catatan

- GPIO2 adalah LED biru bawaan di banyak board devkit. Untuk LED eksternal,
  pakai resistor 220–330 Ω ke GND dan ganti `LED_PIN` bila perlu.
- DHT11/DHT22 butuh pull-up 10 kΩ dari pin data ke 3V3.
- Server Flask harus dijalankan dengan `--host 0.0.0.0` agar bisa diakses dari
  ESP32, dan port 5000 harus diizinkan di Windows Firewall.
