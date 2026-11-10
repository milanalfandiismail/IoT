# Desain: Dashboard Bertipe Device + Akun di Database

**Tanggal:** 2026-11-10
**Status:** Menunggu persetujuan pemilik proyek
**Melanjutkan:** `2026-11-10-iot-led-design.md` (revisi 3)

## Tujuan

Dashboard sekarang hanya satu saklar LED. Targetnya: tampilan yang terasa seperti
produk, di mana setiap device punya tipe dan kartunya menyesuaikan:

- **LED** → switch ON/OFF
- **Suhu** → angka suhu terakhir + waktu pembacaan

Ditambah: akun login dikelola dari web (CRUD), bukan dari `.env`, sehingga tiap
anggota kelompok punya login sendiri — tetapi semua melihat dan mengontrol
device yang sama.

## Alasan perubahan akun

`.env` hanya muat satu admin, dan mengganti password berarti edit file + restart
server. Untuk demo kelompok, akun perlu dibuat/dihapus tanpa menyentuh server.
`SECRET_KEY` dan `API_TOKEN` (untuk `curl`) tetap di `.env` karena itu rahasia
infrastruktur, bukan akun.

## Pendekatan yang dipertimbangkan

| Pendekatan | Keputusan |
|---|---|
| Suhu: angka + grafik histori | **Ditolak untuk sekarang.** Butuh tabel `readings` + chart; nilai tambahnya kecil dibanding waktu. Dipilih: suhu terakhir di kolom device. Bisa ditambah nanti tanpa membongkar model. |
| Suhu: angka besar + "x detik lalu" | **Dipilih.** Satu kolom `temperature` + `updated_at`. |
| Akun: semua user setara | Ditolak. Siapa pun bisa menghapus akun siapa pun; tidak ada yang bisa membetulkan akun yang salah. |
| Akun: seed admin lewat CLI | Ditolak. Menambah langkah setup yang mudah lupa. |
| Akun: halaman setup pertama + peran admin | **Dipilih (versi akhir).** Tidak ada halaman setup: saat tidak ada user admin di DB, server otomatis membuat `admin` / `rahasia`. Admin mengelola user; user biasa hanya mengontrol device dan mengganti password sendiri. |
| UI: `/devices` terpisah dari dashboard | Ditolak. Dua halaman terasa seperti alat internal. Dikelola langsung di dashboard. |
| UI: Tailwind CDN | Ditolak. Demo harus jalan tanpa internet. CSS ditulis sendiri di `static/style.css`. |
| Satu board dengan LED + DHT | Dua device (`led-...`, `suhu-...`), dua token, dipakai satu sketch. Tidak menambah model data. |

## Model data (SQLite `data.db`)

```sql
CREATE TABLE devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type TEXT NOT NULL CHECK (type IN ('led', 'suhu')),
    token_hash TEXT NOT NULL UNIQUE,
    token TEXT,
    enabled INTEGER NOT NULL DEFAULT 1,
    led_on INTEGER NOT NULL DEFAULT 0,
    temperature REAL,
    updated_at TEXT
);

CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE COLLATE NOCASE,
    password_hash TEXT NOT NULL,
    is_admin INTEGER NOT NULL DEFAULT 0,
    created_at TEXT NOT NULL
);
```

Migrasi DB lama dijalankan otomatis saat koneksi: `PRAGMA table_info` lalu
`ALTER TABLE ... ADD COLUMN` untuk `type` (default `'led'`), `led_on`,
`temperature`, `updated_at`; tabel `users` dibuat dengan `CREATE TABLE IF NOT
EXISTS`.

State LED **pindah dari variabel global ke kolom `led_on`**. Tanpa itu dua kartu
LED akan bergerak bersamaan. Efek samping yang diterima: state bertahan setelah
restart server.

## Endpoint

| Method | Path | Auth | Body / Hasil |
|---|---|---|---|
| `GET` | `/login` | publik | Form login |
| `POST` | `/login` | publik | Cek ke tabel `users`; salah → 401 |
| `POST` | `/logout` | session | Hapus session |
| `GET` | `/` | session | Dashboard: grid kartu semua device + form tambah device |
| `GET` | `/akun` | session | Ganti password sendiri; admin juga melihat daftar user |
| `POST` | `/akun/password` | session | `password_lama`, `password_baru` → ganti password sendiri |
| `POST` | `/akun/users` | admin | `username`, `password`, `is_admin` → tambah user |
| `POST` | `/akun/users/<id>/password` | admin | Reset password user |
| `POST` | `/akun/users/<id>/delete` | admin | Hapus user (tidak boleh diri sendiri) |
| `POST` | `/devices` | session | `name`, `type` → device baru + token ditampilkan |
| `POST` | `/devices/<id>/delete` | session | Hapus device |
| `GET` | `/api/devices` | session | JSON untuk refresh 2 detik: `[{id,name,type,on,temperature,updated_at}]` |
| `POST` | `/api/devices/<id>/led` | session atau `Bearer API_TOKEN` | `{"on": bool}` → 200 `{"on": bool}` |
| `POST` | `/api/device/temperature` | `Bearer <token device bertipe suhu>` | `{"celsius": 27.4}` → 200 |
| `GET` | `/api/device/state` | `Bearer <token device bertipe led>` | `{"on": bool}` |

### Aturan validasi

- `type` wajib `led` atau `suhu`; selain itu → 400.
- `celsius` wajib number (bukan bool), rentang `-50..80`; di luar itu → 400.
- Suhu hanya boleh dikirim oleh device bertipe `suhu`; device LED → 400.
- Polling state hanya untuk device bertipe `led`; device suhu → 400.
- `POST /api/devices/<id>/led` ke device bertipe suhu → 400.
- Username: 3–32 karakter, hanya huruf/angka/`_`/`-`, unik (case-insensitive).
- Password: minimal 6 karakter. Hash `werkzeug`.
- Saat tidak ada user dengan `is_admin = 1`, server membuat `admin` / `rahasia`
  (password default demo; ganti lewat `/akun`). Jika username `admin` sudah
  dipakai user biasa, user itu dinaikkan menjadi admin.
- Admin tidak bisa menghapus akunnya sendiri (mencegah terkunci di luar).
- Session menyimpan `user_id`, `username`, `is_admin`. `is_admin` dibaca ulang
  dari DB setiap request, bukan dipercaya dari cookie.

## Tampilan

`base.html` + `static/style.css` (satu file, tanpa framework):

```
┌──────────────────────────────────────────────────────────────────────┐
│  IoT Control            [Dashboard] [Akun]   admin ▾   [Keluar]      │
├──────────────────────────────────────────────────────────────────────┤
│  Perangkat                          [+ Tambah device]                │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐                  │
│  │ LED Dapur    │ │ Suhu Ruang   │ │ LED Kamar    │                  │
│  │ [LED]        │ │ [SUHU]       │ │ [LED]        │                  │
│  │   ● ON       │ │  27.4 °C     │ │   ○ OFF      │                  │
│  │ [Matikan]    │ │ 2 detik lalu │ │ [Nyalakan]   │                  │
│  │ Lihat token  │ │ Lihat token  │ │ Lihat token  │                  │
│  │ Cabut        │ │ Cabut        │ │ Cabut        │                  │
│  └──────────────┘ └──────────────┘ └──────────────┘                  │
└──────────────────────────────────────────────────────────────────────┘
```

- Grid responsif (`auto-fill, minmax(260px, 1fr)`). Kartu putih, radius 12,
  bayangan halus, badge tipe berwarna (LED hijau, Suhu biru), status titik
  hijau/abu.
- Token per device di `<details>` "Lihat token" — bisa disalin kapan saja.
- Form tambah device pakai `<details>` (tanpa JS modal, tanpa dependency).
- Refresh data tiap 2 detik lewat `GET /api/devices`; tombol tetap pakai
  `fetch` lalu render ulang. Halaman tidak reload, jadi posisi scroll tidak
  melompat.
- Login dan setup memakai kartu tengah yang sama dengan dashboard secara visual.

## Firmware

Dua sketch, satu per tipe:

- `esp32/led_poll/led_poll.ino` — tetap polling `GET /api/device/state`.
- `esp32/dht_post/dht_post.ino` — baca DHT11/DHT22 (library Adafruit
  `DHT sensor library`), kirim `POST /api/device/temperature` tiap 5 detik.
  Serial print suhu yang dikirim dan kode respons.

## Tes

- `tests/test_auth.py`: login dari DB, login salah, logout, redirect setup.
- `tests/test_accounts.py`: setup pertama, setup ditolak setelah ada user,
  tambah user, username duplikat, reset password, hapus user, admin tidak bisa
  hapus diri sendiri, user biasa tidak bisa akses route admin, ganti password.
- `tests/test_devices.py`: tambah device LED/Suhu, tipe tidak valid, token bisa
  dilihat, cabut.
- `tests/test_api.py`: state per device (dua device LED tidak saling
  mempengaruhi), auth token, `GET /api/devices` hanya session.
- `tests/test_temperature.py`: kirim suhu valid, device LED ditolak, nilai di
  luar rentang ditolak, body bukan number ditolak, token salah 401.
- `tests/test_migration.py`: DB skema lama dimigrasi tanpa kehilangan device.

## Non-goals

- Grafik histori suhu, notifikasi, WebSocket.
- Hak akses per-device (semua user kelompok melihat semua device).
- Lupa password lewat email; reset dilakukan admin.
- HTTPS, rate limit, audit log.
