def test_dashboard_shows_device_cards(logged_in):
    logged_in.post("/devices", data={"name": "led-dapur", "type": "led"})
    logged_in.post("/devices", data={"name": "suhu-ruang", "type": "suhu"})
    page = logged_in.get("/").get_data(as_text=True)
    assert "led-dapur" in page
    assert "suhu-ruang" in page
    assert 'data-type="led"' in page
    assert 'data-type="suhu"' in page


def test_dashboard_has_add_device_form(logged_in):
    page = logged_in.get("/").get_data(as_text=True)
    assert 'name="type"' in page
    assert "Tambah device" in page


def test_akun_has_password_form(logged_in):
    page = logged_in.get("/akun").get_data(as_text=True)
    assert 'name="password_lama"' in page
    assert 'name="password_baru"' in page


def test_admin_sees_user_list(logged_in):
    page = logged_in.get("/akun").get_data(as_text=True)
    assert "Daftar user" in page
    assert "admin" in page


def test_old_devices_page_gone(logged_in):
    assert logged_in.get("/devices").status_code == 405
