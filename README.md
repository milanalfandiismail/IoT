# Kontrol LED ESP32 via Flask

Server Flask + dashboard web untuk menyalakan/mematikan satu LED ESP32 lewat HTTP polling.

```
ESP32  --GET /api/device/state (Bearer token device)-->  Flask   tiap 1 detik
       <-- {"on": true} ------------------------------

Browser --POST /login (session)------+
        --POST /api/led {"on":true}--+--> Flask
```

- ESP32 adalah client: dia yang menembak Flask, jadi Flask tidak perlu tahu IP ESP32.
- LED berubah maksimal ~1 detik setelah perintah dikirim.
- State disimpan di memori; restart server = LED kembali ke `OFF`.

## Setup

### 1. Install dependency

```powershell
python -m venv .venv
.\.venv\Scripts\activate
pip install -r requirements.txt
```

### 2. Buat `.env`

Salin `.env.example` menjadi `.env`, lalu isi:

**Hash password** (yang disimpan hanya hash, bukan password asli):

```powershell
python -c "from werkzeug.security import generate_password_hash; print(generate_password_hash('password-kamu'))"
```

**Token acak** (jalankan dua kali: `SECRET_KEY` dan `API_TOKEN` — keduanya harus berbeda):

```powershell
python -c "import secrets; print(secrets.token_urlsafe(32))"
```

Isi `.env`:

```
SECRET_KEY=<token acak pertama>
ADMIN_USER=admin
ADMIN_PASSWORD_HASH=<hasil generate_password_hash>
API_TOKEN=<token acak kedua>
```

Token device **tidak** ada di `.env` — dibuat dari halaman `/devices` setelah login.

`.env` sudah masuk `.gitignore` — jangan pernah di-commit.

### 3. Jalankan

```powershell
python app.py
```

Server jalan di `0.0.0.0:5000`, jadi bisa diakses ESP32. (Alternatif: `flask --app app run --host 0.0.0.0`.)

Windows akan menanyakan izin firewall untuk Python — pilih **Allow** (jaringan Private).

Buka `http://127.0.0.1:5000`, login dengan `ADMIN_USER` dan password yang kamu
hash tadi. Dashboard punya tombol ON/OFF dan tombol Keluar.

### 4. Tambah device

Buka **Kelola device** di dashboard (atau `/devices`), isi nama (misal
`led-dapur`), lalu **Tambah device**. Token muncul **sekali** di halaman itu —
salin ke sketch ESP32 (`TOKEN`). Server hanya menyimpan hash-nya, jadi token
tidak bisa dilihat lagi: kalau hilang, cabut device dan buat ulang.

Daftar device tersimpan di `data.db` (dibuat otomatis, sudah git-ignored).
Menekan **Cabut** membuat token device itu langsung tidak berlaku.

### 5. Tes

```powershell
pytest -v
```

28 tes: login/session, CRUD device + hash token, validasi body, pemisahan
`API_TOKEN` dari token device, dan skema `bearer` huruf kecil.

### 6. Kontrol lewat terminal (opsional)

```powershell
curl.exe -X POST http://127.0.0.1:5000/api/led -H "Authorization: Bearer <API_TOKEN>" -H "Content-Type: application/json" -d "{\"on\":true}"
```

Cek state seperti yang dilihat ESP32 (pakai token dari halaman `/devices`):

```powershell
curl.exe http://127.0.0.1:5000/api/device/state -H "Authorization: Bearer <token-device>"
```

### 7. Firmware ESP32

Lihat [`esp32/README.md`](esp32/README.md). Yang perlu diubah: SSID, password
Wi-Fi, IP laptop (`ipconfig`), dan `TOKEN` = token device dari halaman `/devices`.

## Endpoint

| Method | Path | Auth | Body / Hasil |
|---|---|---|---|
| `GET` | `/login` | publik | Form login |
| `POST` | `/login` | publik | `username`, `password` → 302 ke `/`, salah → 401 |
| `POST` | `/logout` | session | Hapus session |
| `GET` | `/` | session | Dashboard |
| `GET` | `/devices` | session | Daftar device + form tambah |
| `POST` | `/devices` | session | `name` → halaman menampilkan token sekali; nama kosong → 400 |
| `POST` | `/devices/<id>/delete` | session | Cabut device → 302 |
| `POST` | `/api/led` | session atau `Bearer API_TOKEN` | `{"on": true\|false}` → `{"on": bool}` |
| `GET` | `/api/device/state` | hanya token device dari `/devices` | `{"on": bool}` |

`API_TOKEN` tidak berlaku di `/api/device/state`, dan token device tidak
berlaku di `/api/led` / halaman web — token device yang bocor tidak bisa
membuka dashboard.

## Troubleshooting

| Gejala | Penyebab |
|---|---|
| App gagal start dengan `KeyError` atau `RuntimeError` | Ada variabel yang hilang/kosong di `.env` |
| ESP32 `poll gagal: -1` | IP salah, server tidak pakai `--host 0.0.0.0`, atau firewall memblokir port 5000 |
| ESP32 `poll gagal: 401` | `TOKEN` di sketch bukan token dari `/devices`, atau device sudah dicabut |
| Login selalu 401 | `ADMIN_PASSWORD_HASH` bukan hasil `generate_password_hash`, atau password salah |
| Tombol tidak berubah | Session kedaluwarsa (ganti `SECRET_KEY`), login ulang |
| Token device hilang | Tidak bisa dilihat lagi (hanya hash tersimpan): cabut lalu tambah device baru |
