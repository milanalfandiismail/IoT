import pytest


def test_led_requires_auth(client):
    assert client.post("/api/led", json={"on": True}).status_code == 401


def test_led_with_api_token(client):
    r = client.post("/api/led", json={"on": True},
                    headers={"Authorization": "Bearer token-manusia"})
    assert r.status_code == 200
    assert r.get_json() == {"on": True}


@pytest.mark.parametrize("payload", [{"on": 1}, {"on": "true"}, {}, None, []])
def test_led_rejects_invalid_payload(client, payload):
    r = client.post("/api/led", json=payload,
                    headers={"Authorization": "Bearer token-manusia"})
    assert r.status_code == 400


def test_led_rejects_non_json_body(client):
    r = client.post("/api/led", data="on=true",
                    headers={"Authorization": "Bearer token-manusia"})
    assert r.status_code == 400


def test_led_rejects_device_token(client):
    r = client.post("/api/led", json={"on": True},
                    headers={"Authorization": "Bearer token-esp32"})
    assert r.status_code == 401


def test_device_state_with_device_token(client):
    client.post("/api/led", json={"on": True},
                headers={"Authorization": "Bearer token-manusia"})
    r = client.get("/api/device/state",
                   headers={"Authorization": "Bearer token-esp32"})
    assert r.status_code == 200
    assert r.get_json() == {"on": True}


def test_device_state_rejects_api_token(client):
    r = client.get("/api/device/state",
                   headers={"Authorization": "Bearer token-manusia"})
    assert r.status_code == 401


def test_bearer_scheme_is_case_insensitive(client):
    r = client.get("/api/device/state",
                   headers={"Authorization": "bearer token-esp32"})
    assert r.status_code == 200


def test_non_ascii_token_is_401(client):
    r = client.get("/api/device/state",
                   headers={"Authorization": "Bearer tok\u00e9"})
    assert r.status_code == 401
