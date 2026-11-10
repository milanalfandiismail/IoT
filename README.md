# Kontrol LED ESP32 via Flask

Server Flask + dashboard web untuk menyalakan/mematikan satu LED ESP32 lewat HTTP polling.

```
ESP32  --GET /api/device/state (Bearer DEVICE_TOKEN)-->  Flask   tiap 1 detik
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

**Token acak** (jalankan tiga kali: `SECRET_KEY`, `API_TOKEN`, `DEVICE_TOKEN` — ketiganya harus berbeda):

```powershell
python -c "import secrets; print(secrets.token_urlsafe(32))"
```

Isi `.env`:

```
SECRET_KEY=<token acak pertama>
ADMIN_USER=admin
ADMIN_PASSWORD_HASH=<hasil generate_password_hash>
API_TOKEN=<token acak kedua>
DEVICE_TOKEN=<token acak ketiga>
```

`.env` sudah masuk `.gitignore` — jangan pernah di-commit.

### 3. Jalankan

```powershell
flask --app app run --host 0.0.0.0
```

`--host 0.0.0.0` wajib agar ESP32 bisa connect. Windows akan menanyakan izin
firewall untuk Python — pilih **Allow** (jaringan Private).

Buka `http://127.0.0.1:5000`, login dengan `ADMIN_USER` dan password yang kamu
hash tadi. Dashboard punya tombol ON/OFF dan tombol Keluar.

### 4. Tes

```powershell
pytest -v
```

19 tes: login/session, pemisahan `API_TOKEN` vs `DEVICE_TOKEN`, validasi body,
dan skema `bearer` huruf kecil.

### 5. Kontrol lewat terminal (opsional)

```powershell
curl.exe -X POST http://127.0.0.1:5000/api/led -H "Authorization: Bearer <API_TOKEN>" -H "Content-Type: application/json" -d "{\"on\":true}"
```

Cek state seperti yang dilihat ESP32:

```powershell
curl.exe http://127.0.0.1:5000/api/device/state -H "Authorization: Bearer <DEVICE_TOKEN>"
```

### 6. Firmware ESP32

Lihat [`esp32/README.md`](esp32/README.md). Yang perlu diubah: SSID, password
Wi-Fi, IP laptop (`ipconfig`), dan `TOKEN` = `DEVICE_TOKEN`.

## Endpoint

| Method | Path | Auth | Body / Hasil |
|---|---|---|---|
| `GET` | `/login` | publik | Form login |
| `POST` | `/login` | publik | `username`, `password` → 302 ke `/`, salah → 401 |
| `POST` | `/logout` | session | Hapus session |
| `GET` | `/` | session | Dashboard |
| `POST` | `/api/led` | session atau `Bearer API_TOKEN` | `{"on": true\|false}` → `{"on": bool}` |
| `GET` | `/api/device/state` | hanya `Bearer DEVICE_TOKEN` | `{"on": bool}` |

`API_TOKEN` tidak berlaku di `/api/device/state`, dan `DEVICE_TOKEN` tidak
berlaku di `/api/led` — token device yang bocor tidak bisa membuka dashboard.

## Troubleshooting

| Gejala | Penyebab |
|---|---|
| App gagal start dengan `KeyError` atau `RuntimeError` | Ada variabel yang hilang/kosong di `.env`, atau `API_TOKEN` sama dengan `DEVICE_TOKEN` |
| ESP32 `poll gagal: -1` | IP salah, server tidak pakai `--host 0.0.0.0`, atau firewall memblokir port 5000 |
| ESP32 `poll gagal: 401` | `TOKEN` di sketch ≠ `DEVICE_TOKEN` di `.env` |
| Login selalu 401 | `ADMIN_PASSWORD_HASH` bukan hasil `generate_password_hash`, atau password salah |
| Tombol tidak berubah | Session kedaluwarsa (ganti `SECRET_KEY`), login ulang |
