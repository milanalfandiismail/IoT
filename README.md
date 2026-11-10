# Dashboard IoT ESP32 via Flask

Server Flask + dashboard web untuk mengontrol LED dan melihat suhu dari ESP32
lewat HTTP. Setiap device punya tipe (LED / Suhu) dan kartunya menyesuaikan.

```
ESP32 LED --GET  /api/device/state (Bearer token device)--------+
ESP32 DHT --POST /api/device/temperature (Bearer token device)--+--> Flask + data.db
Browser   --POST /api/devices/<id>/led (session)----------------+
```

- ESP32 adalah client: dia yang menembak Flask, jadi Flask tidak perlu tahu IP ESP32.
- LED berubah maksimal ~1 detik setelah perintah dikirim.
- Semua device dan akun tersimpan di `data.db` (dibuat otomatis saat server jalan).
- State LED **tidak** hilang saat server restart.

## Setup

### 1. Install dependency

```powershell
python -m venv .venv
.\.venv\Scripts\activate
pip install -r requirements.txt
```

### 2. Buat `.env`

Salin `.env.example` menjadi `.env`. Isinya hanya dua rahasia server:

```
SECRET_KEY=<token acak pertama>
API_TOKEN=<token acak kedua>
```

Buat token acak (jalankan dua kali, keduanya harus berbeda):

```powershell
python -c "import secrets; print(secrets.token_urlsafe(32))"
```

`API_TOKEN` dipakai untuk mengontrol LED lewat `curl`, bukan untuk login.
`.env` sudah masuk `.gitignore` — jangan pernah di-commit.

### 3. Jalankan

```powershell
python app.py
```

Server jalan di `0.0.0.0:5000`, jadi bisa diakses ESP32. Windows akan
menanyakan izin firewall untuk Python — pilih **Allow** (jaringan Private).

Buka `http://127.0.0.1:5000`.

Akun admin dibuat otomatis kalau belum ada admin di database:

| Username | Password |
|---|---|
| `admin` | `rahasia` |

Segera ganti lewat menu **Akun → Ganti password**.

### 4. Kelola akun

Semua anggota kelompok bisa login sendiri dari menu **Akun**:

- **Admin** menambah user, me-reset password user, dan menghapus user.
- Setiap user bisa mengganti password sendiri.
- Semua user melihat dan mengontrol device yang sama — data tidak dipisah per akun.
- Admin tidak bisa menghapus akunnya sendiri.

### 5. Tambah device

Di dashboard tekan **+ Tambah device**, isi nama, pilih tipe:

- **LED** — kartu menampilkan switch ON/OFF.
- **Suhu** — kartu menampilkan suhu terakhir dan kapan dibaca.

Token device ditampilkan di kartu (**Lihat token**) dan bisa disalin kapan saja.
Menekan **Cabut device** membuat token itu langsung tidak berlaku.

Satu board yang punya LED *dan* sensor: tambahkan dua device (`led-...` dan
`suhu-...`), lalu pakai dua token itu di sketch.

Beberapa LED di satu board: tambahkan satu device bertipe **LED** per lampu, lalu
pakai sketch `esp32/led_multi_https_http` (TLS terverifikasi) atau
`esp32/led_multi_https_insecure` (demo, tanpa verifikasi sertifikat), dan isi
satu baris `lampu[]` per LED (token + nomor pin).

### 6. Tes

```powershell
pytest -q
```

63 tes: login/session, akun admin & anggota, tipe device, state LED per device,
endpoint suhu, validasi body, migrasi `data.db` lama, dan skema `bearer`.

### 7. Kontrol lewat terminal (opsional)

Nyalakan LED dengan `API_TOKEN` (ganti `<id>` dengan id device dari dashboard):

```powershell
curl.exe -X POST http://127.0.0.1:5000/api/devices/1/led -H "Authorization: Bearer <API_TOKEN>" -H "Content-Type: application/json" -d "{\"on\":true}"
```

Cek state seperti yang dilihat ESP32 (pakai token device bertipe LED):

```powershell
curl.exe http://127.0.0.1:5000/api/device/state -H "Authorization: Bearer <token-device>"
```

Kirim suhu seperti yang dilakukan ESP32 (token device bertipe Suhu):

```powershell
curl.exe -X POST http://127.0.0.1:5000/api/device/temperature -H "Authorization: Bearer <token-device>" -H "Content-Type: application/json" -d "{\"celsius\":27.4}"
```

### 8. Firmware ESP32

Lihat [`esp32/README.md`](esp32/README.md). Yang perlu diubah: SSID, password
Wi-Fi, IP laptop (`ipconfig`), dan `TOKEN` = token device dari kartu dashboard.

## Endpoint

| Method | Path | Auth | Body / Hasil |
|---|---|---|---|
| `GET` | `/login` | publik | Form login |
| `POST` | `/login` | publik | `username`, `password` → 302 ke `/`, salah → 401 |
| `POST` | `/logout` | session | Hapus session |
| `GET` | `/` | session | Dashboard kartu device |
| `GET` | `/akun` | session | Ganti password; admin melihat daftar user |
| `POST` | `/akun/password` | session | `password_lama`, `password_baru` |
| `POST` | `/akun/users` | admin | `username`, `password`, `is_admin` |
| `POST` | `/akun/users/<id>/password` | admin | Reset password user |
| `POST` | `/akun/users/<id>/delete` | admin | Hapus user |
| `POST` | `/devices` | session | `name`, `type` (`led`/`suhu`) → token di dashboard |
| `POST` | `/devices/<id>/delete` | session | Cabut device |
| `GET` | `/api/devices` | session | JSON semua device untuk refresh dashboard |
| `POST` | `/api/devices/<id>/led` | session atau `Bearer API_TOKEN` | `{"on": true\|false}` |
| `GET` | `/api/device/state` | token device LED | `{"on": bool}` |
| `POST` | `/api/device/temperature` | token device Suhu | `{"celsius": angka -50..80}` |

`API_TOKEN` tidak berlaku di endpoint token device, dan token device tidak
berlaku di halaman web — token device yang bocor tidak bisa membuka dashboard.

## Troubleshooting

| Gejala | Penyebab |
|---|---|
| App gagal start dengan `RuntimeError` | `SECRET_KEY` atau `API_TOKEN` kosong di `.env` |
| ESP32 `poll gagal: -1` | IP salah, server tidak jalan di `0.0.0.0`, atau firewall memblokir port 5000 |
| ESP32 `poll gagal: 401` | `TOKEN` di sketch bukan token device, atau device sudah dicabut |
| ESP32 `POST 400` | Kirim suhu ke device bertipe LED, atau nilai di luar `-50..80` |
| Login selalu 401 | Password salah, atau user sudah dihapus admin |
| Tombol tidak berubah | Session kedaluwarsa (ganti `SECRET_KEY`), login ulang |
| Suhu tidak muncul | Sketch `dht_post` belum jalan, token salah, atau device bertipe LED |
| Butuh reset total | Hentikan server, hapus `data.db`; admin/rahasia dibuat lagi saat start |
