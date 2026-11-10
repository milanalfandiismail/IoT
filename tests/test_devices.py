import re
import sqlite3

from app import add_device, db_path, device_from_token, list_devices


def test_add_device_returns_usable_token():
    token = add_device("led-dapur")
    assert device_from_token(token)["name"] == "led-dapur"


def test_unknown_token_not_found():
    add_device("led-dapur")
    assert device_from_token("token-palsu") is None


def test_database_stores_hash_not_token():
    token = add_device("led-dapur")
    rows = sqlite3.connect(db_path()).execute("SELECT token_hash FROM devices").fetchall()
    assert len(rows) == 1
    assert rows[0][0] != token
    assert len(rows[0][0]) == 64


def test_devices_page_requires_login(client):
    assert client.get("/devices").status_code == 302


def test_add_device_shows_token_once(logged_in):
    r = logged_in.post("/devices", data={"name": "led-dapur"})
    assert r.status_code == 200
    token = re.search(r'id="token">([^<]+)<', r.get_data(as_text=True)).group(1)
    assert device_from_token(token)["name"] == "led-dapur"
    assert token not in logged_in.get("/devices").get_data(as_text=True)


def test_add_device_rejects_empty_name(logged_in):
    assert logged_in.post("/devices", data={"name": "   "}).status_code == 400
    assert list_devices() == []


def test_revoke_device_blocks_polling(logged_in, client, device_token):
    device_id = device_from_token(device_token)["id"]
    assert logged_in.post(f"/devices/{device_id}/delete").status_code == 302
    r = client.get("/api/device/state",
                   headers={"Authorization": f"Bearer {device_token}"})
    assert r.status_code == 401
