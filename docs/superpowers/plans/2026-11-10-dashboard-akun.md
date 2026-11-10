# Dashboard Bertipe + Akun DB — Rencana Implementasi

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Dashboard kartu per device (tipe LED / Suhu) dengan akun login yang dikelola dari web, semua data di SQLite.

**Architecture:** `app.py` tetap satu modul (ukuran masih wajar). State LED pindah ke kolom `led_on`. Auth sesi memakai tabel `users`; admin otomatis dibuat saat tidak ada admin. Dashboard me-refresh `GET /api/devices` tiap 2 detik.

**Tech Stack:** Flask 3, sqlite3 (stdlib), Werkzeug security, pytest. Tanpa dependency baru.

**Spec:** `docs/superpowers/specs/2026-11-10-dashboard-akun-design.md`

## Global Constraints

- Komentar dan teks UI berbahasa Indonesia; identitas kode berbahasa Inggris.
- Tanpa dependency baru; `sqlite3`, `hashlib`, `secrets`, `hmac` sudah stdlib.
- `.env` hanya `SECRET_KEY` dan `API_TOKEN`.
- Token device: 32 byte `secrets.token_urlsafe`, disimpan plaintext (untuk ditampilkan) + SHA-256 (untuk lookup).
- Suhu rentang `-50..80`, tolak bool.
- Username 3–32 karakter `[A-Za-z0-9_-]+`, unik tanpa peduli huruf besar/kecil.
- Password minimal 6 karakter.
- Admin default `admin` / `rahasia` dibuat saat `SELECT ... WHERE is_admin = 1` kosong.
- Admin tidak bisa menghapus dirinya sendiri.

## Review Focus

1. Dua device LED harus punya state terpisah (bug paling mungkin: masih memakai satu variabel).
2. Device suhu tidak boleh bisa dipanggil `GET /api/device/state`, dan device LED tidak boleh mengirim suhu.
3. Token device yang salah/`Authorization` non-ASCII tidak boleh 500.
4. User biasa tidak boleh bisa membuka/menembak route admin lewat POST langsung.
5. Migrasi `data.db` lama tidak boleh menghapus device atau menggagalkan startup.

---

### Task 1: Skema, migrasi, seed admin, fungsi data

**Files:** Modify `app.py` (bagian atas sampai sebelum `create_app`). Test: `tests/test_migration.py`, `tests/test_accounts.py`.

**Interfaces:**
- Produces: `db_path() -> str`, `_db()` context manager, `hash_token(token: str) -> str`,
  `add_device(name: str, device_type: str) -> str`, `list_devices() -> list[dict]`,
  `device_from_token(token: str) -> dict | None`, `delete_device(device_id: int) -> None`,
  `set_led(device_id: int, on: bool) -> None`, `record_temperature(device_id: int, celsius: float) -> None`,
  `dashboard_devices() -> list[dict]`, `find_user(username: str) -> dict | None`,
  `user_by_id(user_id: int) -> dict | None`, `add_user(username: str, password: str, is_admin: bool) -> int`,
  `list_users() -> list[dict]`, `set_password(user_id: int, password: str) -> None`,
  `delete_user(user_id: int) -> None`, `valid_username(name: str) -> bool`.

- [ ] **Step 1: Tulis tes migrasi** — buat DB dengan skema lama (`id,name,token_hash,token,enabled`), isi satu device, lalu panggil `list_devices()`; harapkan device masih ada dengan `type == "led"` dan `temperature is None`.
- [ ] **Step 2: Jalankan, pastikan gagal** (`KeyError: 'type'` / kolom tidak ada).
- [ ] **Step 3: Implementasi skema + migrasi** — `SCHEMA` baru untuk `devices` dan `users`; di `_db()` cek `PRAGMA table_info(devices)` lalu `ALTER TABLE ... ADD COLUMN` untuk `type` (default `'led'`), `led_on`, `temperature`, `updated_at`. Setelahnya `_seed_admin(conn)`: kalau tidak ada baris `is_admin = 1`, insert `admin`/`hash("rahasia")`; kalau username `admin` sudah ada, `UPDATE users SET is_admin = 1`.
- [ ] **Step 4: Tulis tes akun dasar** — `find_user("ADMIN")` mengembalikan baris admin (case-insensitive), `set_password` lalu `check_password_hash` benar, `delete_user`, `add_user` duplikat memicu `sqlite3.IntegrityError`.
- [ ] **Step 5: Implementasi fungsi** di atas; `dashboard_devices()` mengembalikan `[{"id","name","type","on","temperature","updated_at"}]`, `on = bool(led_on)`.
- [ ] **Step 6: `pytest -q` hijau, commit** `feat: skema device bertipe dan tabel users`.

### Task 2: Login dari DB, guard admin, halaman akun

**Files:** Modify `app.py` route login/logout + route akun. Test: `tests/test_auth.py`, `tests/test_accounts.py`.

**Interfaces:**
- Consumes: Task 1.
- Produces: `require_session`, `require_admin`, `current_user() -> dict | None` (baca DB via `session["user_id"]`), session keys `user_id`, `username`, `is_admin`.

- [ ] **Step 1: Tulis tes** — login `admin`/`rahasia` → 302; password salah → 401; `logged_in` bisa `GET /`; user non-admin `GET /akun/users` → 403; admin tambah user → baris DB ada; username duplikat → 400; admin hapus diri sendiri → 400; reset password user lalu login dengan password baru → 302; ganti password sendiri (password lama salah → 400).
- [ ] **Step 2: Jalankan, pastikan gagal.**
- [ ] **Step 3: Implementasi** route `GET/POST /login` (cari `find_user`, `check_password_hash`), `POST /logout`, `GET /akun`, `POST /akun/password`, `POST /akun/users`, `POST /akun/users/<int:user_id>/password`, `POST /akun/users/<int:user_id>/delete`. `require_admin` mengembalikan 403 jika `current_user()["is_admin"]` false.
- [ ] **Step 4: `pytest -q` hijau, commit** `feat: akun login dari database, admin kelola user`.

### Task 3: Device bertipe, dashboard JSON, LED per device

**Files:** Modify `app.py`. Test: `tests/test_devices.py`, `tests/test_api.py`.

**Interfaces:**
- Consumes: Task 1.
- Produces: route `GET /`, `GET /api/devices`, `POST /devices`, `POST /devices/<int:device_id>/delete`, `POST /api/devices/<int:device_id>/led`, `GET /api/device/state`.

- [ ] **Step 1: Tulis tes** — tambah device LED dan Suhu (`type` tersimpan, token 43 karakter, bisa dilihat); `type="xyz"` → 400; dua device LED: nyalakan yang pertama → yang kedua tetap `False`; `/api/devices` tanpa session → 302, dengan session → JSON tanpa `token_hash`; `/api/devices/<id>/led` ke device suhu → 400; body `{"on": 1}` → 400.
- [ ] **Step 2: Jalankan, pastikan gagal.**
- [ ] **Step 3: Implementasi** — `POST /devices` validasi `name` dan `device_type in ("led", "suhu")`; `set_led` cuma `UPDATE devices SET led_on = ?, updated_at = ? WHERE id = ? AND type = 'led'`, return 400 bila 0 baris berubah; `GET /api/device/state` lookup token, tolak jika `type != "led"`; logging perubahan state + poll.
- [ ] **Step 4: `pytest -q` hijau, commit** `feat: device bertipe, state LED per device`.

### Task 4: Endpoint suhu

**Files:** Modify `app.py`. Test: `tests/test_temperature.py`.

**Interfaces:** Produces `POST /api/device/temperature`.

- [ ] **Step 1: Tulis tes** — device suhu kirim `{"celsius": 27.4}` → 200 dan `dashboard_devices()[0]["temperature"] == 27.4`; `{"celsius": true}` → 400; `{"celsius": 81}` → 400; device LED kirim suhu → 400; tanpa token → 401; token salah/non-ASCII → 401 bukan 500.
- [ ] **Step 2: Jalankan, pastikan gagal.**
- [ ] **Step 3: Implementasi** — validasi `isinstance(celsius, (int, float)) and not isinstance(celsius, bool)` dan `-50 <= celsius <= 80`; lookup token; `record_temperature` menulis `temperature` + `updated_at` (UTC ISO, `datetime.now(timezone.utc).isoformat(timespec="seconds")`); log bacaan.
- [ ] **Step 4: `pytest -q` hijau, commit** `feat: endpoint suhu per device`.

### Task 5: UI dashboard dan halaman akun

**Files:** Create `templates/base.html`, `templates/akun.html`, `static/style.css`; rewrite `templates/index.html`, `templates/login.html`; delete `templates/devices.html`. Test: `tests/test_dashboard.py`.

**Interfaces:** Consumes semua route Task 2–4. Halaman memakai `devices`, `user`, `users`, `new_token`, `error`.

- [ ] **Step 1: Tulis tes** — `/` memuat kartu nama device dan badge `LED`/`Suhu`; tombol tambah device ada; halaman `/akun` memuat form ganti password; admin melihat daftar user.
- [ ] **Step 2: Jalankan, pastikan gagal.**
- [ ] **Step 3: Implementasi template + CSS** sesuai sketsa spec: `base.html` (nav, blok `content`), grid `auto-fill minmax(260px,1fr)`, badge tipe, `<details>` token, form tambah device bertipe, script refresh `GET /api/devices` tiap 2000 ms yang merender ulang grid dan memasang ulang handler tombol.
- [ ] **Step 4: `pytest -q` hijau, commit** `feat: dashboard kartu dan halaman akun`.

### Task 6: Firmware suhu, README, `.env.example`

**Files:** Create `esp32/dht_post/dht_post.ino`; modify `README.md`, `.env.example`; Test: jalankan suite penuh.

- [ ] **Step 1: Tulis sketch** — `DHT dht(DHT_PIN, DHT22)`, baca tiap 5 detik, `POST /api/device/temperature` dengan header Bearer, `setTimeout(2000)`, `http.end()` di semua jalur, print hasil.
- [ ] **Step 2: Perbarui README** — login `admin`/`rahasia` + wajib ganti, alur tambah device & pilih tipe, dua sketch, curl login sesi, endpoint suhu, troubleshooting lama.
- [ ] **Step 3: `.env.example` hanya `SECRET_KEY` + `API_TOKEN`.**
- [ ] **Step 4: `pytest -q` hijau, commit** `docs: alur dashboard v2 dan sketch DHT`.
