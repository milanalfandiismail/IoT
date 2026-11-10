# Firmware ESP32

`led_poll/led_poll.ino` melakukan polling `GET /api/device/state` tiap 1 detik
dan menyalakan LED di GPIO2 sesuai respons `{"on": true|false}`.

## Yang harus diubah sebelum upload

1. `SSID` dan `PASSWORD` — Wi-Fi yang sama dengan laptop server.
2. `URL` — ganti `192.168.1.10` dengan IP laptop hasil `ipconfig` (IPv4 Address).
   Pakai IP LAN, bukan `127.0.0.1`.
3. `TOKEN` — token device dari halaman `/devices` (login dulu di dashboard,
   klik **Kelola device**, tambah device, salin token yang muncul sekali itu).

## Upload

1. Board Manager: pasang paket **esp32** (Espressif Systems).
2. Buka `led_poll.ino`, pilih board **ESP32 Dev Module**, port COM yang muncul.
3. Upload, lalu buka Serial Monitor **115200**.

## Yang seharusnya terlihat

- Titik-titik saat menyambung Wi-Fi, lalu `WiFi tersambung`.
- LED mengikuti tombol di dashboard dalam maksimal ~1 detik.
- Server dimatikan → `poll gagal: -1`, LED tetap di state terakhir.
- Server dihidupkan lagi → LED menyamakan diri dalam 1 detik.

## Catatan

- GPIO2 adalah LED biru bawaan di banyak board devkit. Untuk LED eksternal,
  pakai resistor 220–330 Ω ke GND dan ganti `LED_PIN` bila perlu.
- Server Flask harus dijalankan dengan `--host 0.0.0.0` agar bisa diakses dari
  ESP32, dan port 5000 harus diizinkan di Windows Firewall.
