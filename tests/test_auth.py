from werkzeug.security import check_password_hash

from app import find_user


def test_index_requires_login(client):
    r = client.get("/")
    assert r.status_code == 302
    assert "/login" in r.headers["Location"]


def test_login_success(logged_in):
    r = logged_in.get("/")
    assert r.status_code == 200


def test_login_wrong_password(client):
    r = client.post("/login", data={"username": "admin", "password": "salah"})
    assert r.status_code == 401


def test_login_username_case_insensitive(client):
    r = client.post("/login", data={"username": "ADMIN", "password": "rahasia"})
    assert r.status_code == 302


def test_default_admin_seeded(client):
    user = find_user("admin")
    assert user is not None
    assert user["is_admin"] == 1
    assert check_password_hash(user["password_hash"], "rahasia")


def test_logout(logged_in):
    logged_in.post("/logout")
    assert logged_in.get("/").status_code == 302
