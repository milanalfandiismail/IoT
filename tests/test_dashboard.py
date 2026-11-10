def test_dashboard_requires_login(client):
    assert client.get("/").status_code == 302


def test_dashboard_shows_state(logged_in):
    r = logged_in.get("/")
    assert r.status_code == 200
    assert b"LED" in r.data
    assert b"OFF" in r.data


def test_led_via_session_cookie(logged_in):
    r = logged_in.post("/api/led", json={"on": True})
    assert r.status_code == 200
    assert b"ON" in logged_in.get("/").data
