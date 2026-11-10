import logging

import pytest

from app import list_devices


def test_toggle_requires_auth(client, led):
    device_id = list_devices()[0]["id"]
    r = client.post(f"/api/devices/{device_id}/led", json={"on": True})
    assert r.status_code == 401


def test_two_leds_are_independent(logged_in):
    logged_in.post("/devices", data={"name": "led-a", "type": "led"})
    logged_in.post("/devices", data={"name": "led-b", "type": "led"})
    a, b = list_devices()

    r = logged_in.post(f"/api/devices/{a['id']}/led", json={"on": True})
    assert r.status_code == 200
    assert r.get_json() == {"id": a["id"], "on": True}

    devices = list_devices()
    assert devices[0]["led_on"] == 1
    assert devices[1]["led_on"] == 0


def test_poll_reads_own_device_state(logged_in):
    logged_in.post("/devices", data={"name": "led-a", "type": "led"})
    logged_in.post("/devices", data={"name": "led-b", "type": "led"})
    a, b = list_devices()
    logged_in.post(f"/api/devices/{a['id']}/led", json={"on": True})

    r = logged_in.get(
        "/api/device/state", headers={"Authorization": f"Bearer {a['token']}"}
    )
    assert r.get_json() == {"on": True}
    r = logged_in.get(
        "/api/device/state", headers={"Authorization": f"Bearer {b['token']}"}
    )
    assert r.get_json() == {"on": False}


@pytest.mark.parametrize("payload", [{"on": 1}, {"on": "true"}, {}, None, []])
def test_toggle_rejects_invalid_payload(logged_in, led, payload):
    device_id = list_devices()[0]["id"]
    r = logged_in.post(f"/api/devices/{device_id}/led", json=payload)
    assert r.status_code == 400


def test_toggle_rejects_suhu_device(logged_in, suhu):
    device_id = list_devices()[0]["id"]
    r = logged_in.post(f"/api/devices/{device_id}/led", json={"on": True})
    assert r.status_code == 400


def test_toggle_accepts_api_token(client, led):
    device_id = list_devices()[0]["id"]
    r = client.post(
        f"/api/devices/{device_id}/led",
        json={"on": True},
        headers={"Authorization": "Bearer token-manusia"},
    )
    assert r.status_code == 200


def test_devices_json_requires_session(client):
    assert client.get("/api/devices").status_code == 302


def test_devices_json_shape(logged_in, led):
    body = logged_in.get("/api/devices").get_json()
    assert body[0]["name"] == "led-1"
    assert body[0]["type"] == "led"
    assert body[0]["on"] is False
    assert body[0]["temperature"] is None
    assert "token" not in body[0]


def test_poll_rejects_api_token(client):
    r = client.get(
        "/api/device/state", headers={"Authorization": "Bearer token-manusia"}
    )
    assert r.status_code == 401


def test_poll_rejects_suhu_device(client, suhu):
    r = client.get(
        "/api/device/state", headers={"Authorization": f"Bearer {suhu}"}
    )
    assert r.status_code == 400


def test_poll_rejects_non_ascii_token(client):
    r = client.get("/api/device/state", headers={"Authorization": "Bearer tok\u00e9"})
    assert r.status_code == 401


def test_bearer_scheme_is_case_insensitive(client, led):
    r = client.get(
        "/api/device/state", headers={"Authorization": f"bearer {led}"}
    )
    assert r.status_code == 200


def test_logs_led_change(logged_in, caplog):
    logged_in.post("/devices", data={"name": "led-dapur", "type": "led"})
    device_id = list_devices()[0]["id"]
    with caplog.at_level(logging.INFO, logger="iot"):
        logged_in.post(f"/api/devices/{device_id}/led", json={"on": True})
    assert "LED led-dapur: ON" in caplog.text


def test_logs_device_poll(client, led, caplog):
    with caplog.at_level(logging.INFO, logger="iot"):
        client.get("/api/device/state", headers={"Authorization": f"Bearer {led}"})
    assert "poll led-1" in caplog.text
