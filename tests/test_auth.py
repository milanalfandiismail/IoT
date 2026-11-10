def test_root_redirects_to_login(client):
    r = client.get("/")
    assert r.status_code == 302
    assert r.headers["Location"].endswith("/login")


def test_login_success_sets_session(client):
    r = client.post("/login", data={"username": "admin", "password": "rahasia"})
    assert r.status_code == 302
    assert r.headers["Location"].endswith("/")
    with client.session_transaction() as s:
        assert s["user"] == "admin"


def test_login_wrong_password(client):
    r = client.post("/login", data={"username": "admin", "password": "salah"})
    assert r.status_code == 401


def test_logout_clears_session(logged_in):
    r = logged_in.post("/logout")
    assert r.status_code == 302
    assert logged_in.get("/").status_code == 302
