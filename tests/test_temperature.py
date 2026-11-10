import pytest

from app import dashboard_devices, list_devices


def kirim(client, token, payload):
    return client.post(
        "/api/device/temperature",
        json=payload,
        headers={"Authorization": f"Bearer {token}"},
    )


def test_temperature_saved(client, suhu):
    r = kirim(client, suhu, {"celsius": 27.4})
    assert r.status_code == 200
    assert dashboard_devices()[0]["temperature"] == 27.4
    assert dashboard_devices()[0]["updated_at"]


@pytest.mark.parametrize("payload", [{"celsius": True}, {"celsius": 81},
                                     {"celsius": -51}, {"celsius": "27"},
                                     {}, None, []])
def test_temperature_rejects_invalid(client, suhu, payload):
    r = client.post(
        "/api/device/temperature",
        json=payload,
        headers={"Authorization": f"Bearer {suhu}"},
    )
    assert r.status_code == 400


def test_temperature_rejects_led_device(client, led):
    assert kirim(client, led, {"celsius": 27.4}).status_code == 400


def test_temperature_requires_token(client):
    assert client.post("/api/device/temperature", json={"celsius": 27.4}).status_code == 401


def test_temperature_rejects_non_ascii_token(client):
    r = client.post(
        "/api/device/temperature",
        json={"celsius": 27.4},
        headers={"Authorization": "Bearer tok\u00e9"},
    )
    assert r.status_code == 401


def test_temperature_does_not_touch_led_state(client, suhu):
    kirim(client, suhu, {"celsius": 30})
    assert list_devices()[0]["led_on"] == 0
