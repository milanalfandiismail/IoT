from app import list_devices


def test_add_led_device(logged_in):
    r = logged_in.post("/devices", data={"name": "led-dapur", "type": "led"})
    assert r.status_code == 302
    device = list_devices()[0]
    assert device["name"] == "led-dapur"
    assert device["type"] == "led"
    assert len(device["token"]) == 43


def test_add_suhu_device(logged_in):
    logged_in.post("/devices", data={"name": "suhu-ruang", "type": "suhu"})
    assert list_devices()[0]["type"] == "suhu"


def test_invalid_type_creates_nothing(logged_in):
    r = logged_in.post("/devices", data={"name": "aneh", "type": "xyz"})
    assert r.status_code == 302
    assert list_devices() == []


def test_empty_name_creates_nothing(logged_in):
    logged_in.post("/devices", data={"name": "  ", "type": "led"})
    assert list_devices() == []


def test_delete_device(logged_in):
    logged_in.post("/devices", data={"name": "led-dapur", "type": "led"})
    device_id = list_devices()[0]["id"]
    r = logged_in.post(f"/devices/{device_id}/delete")
    assert r.status_code == 302
    assert list_devices() == []


def test_token_visible_on_dashboard(logged_in):
    logged_in.post("/devices", data={"name": "led-dapur", "type": "led"})
    token = list_devices()[0]["token"]
    page = logged_in.get("/").get_data(as_text=True)
    assert token in page


def test_delete_requires_login(client):
    r = client.post("/devices/1/delete")
    assert r.status_code == 302
