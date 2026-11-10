# Firmware ESP32

Tiga sketch, pilih sesuai device yang dibuat di dashboard:

- `led_poll/led_poll.ino` — satu LED. Polling `GET /api/device/state` tiap 1
  detik, LED di GPIO2 mengikuti respons `{"on": true|false}`. Pakai token device
  bertipe **LED**.
- `led_multi_https_http/led_multi_https_http.ino` — beberapa LED di satu board,
  HTTPS dengan verifikasi sertifikat (dan tetap bisa `http://`). Butuh `CA_CERT`
  yang cocok dengan server; jam board harus benar.
- `led_multi_https_insecure/led_multi_https_insecure.ino` — sama, tapi tanpa
  verifikasi sertifikat (`setInsecure()`). Untuk demo/tugas di jaringan yang
  tidak bisa diatur. Jangan untuk produksi.
- `dht_post/dht_post.ino` — baca DHT11/DHT22 tiap 5 detik, kirim
  `POST /api/device/temperature`. Pakai token device bertipe **Suhu**.

## Yang harus diubah sebelum upload

1. `SSID` dan `PASSWORD` — Wi-Fi yang sama dengan laptop server.
2. `URL` — ganti `10.10.10.25` dengan IP laptop hasil `ipconfig` (IPv4 Address),
   atau isi domain publik bila server diekspos lewat reverse proxy.
   Pakai IP LAN, bukan `127.0.0.1`.
3. `TOKEN` — token device dari kartu **Lihat token** di dashboard.

Untuk `led_multi_https_http` dan `led_multi_https_insecure`, isi array
`lampu[]`: satu baris per LED berisi label, token, dan nomor GPIO. Board tidak
menyimpan pin di server — kalau kabel pindah pin, ubah baris itu lalu upload
ulang. LED di dashboard tetap satu kartu per baris.

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

`led_multi_https_http` / `led_multi_https_insecure`:

- Sama seperti `led_poll`, tapi baris log menyebut label dan pin:
  `led-2 (GPIO4) ON`.
- Satu token ditolak (`401`) hanya membuat baris itu gagal; LED lain tetap
  mengikuti dashboard.
- Tambah LED = tambah satu baris di `lampu[]` dan satu device di dashboard.

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
- `led_multi_https_http` bisa `http://` maupun `https://`: skema di `URL` yang
  menentukan. Untuk HTTPS, `CA_CERT` di sketch harus berisi root CA server
  (default sudah diisi root CA publik yang umum; ganti bila memakai sertifikat
  sendiri).
- `led_multi_https_insecure` tidak memeriksa sertifikat: koneksi tetap
  terenkripsi, tapi server palsu tidak terdeteksi. Pakai hanya untuk demo.
