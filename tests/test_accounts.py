from app import find_user


def test_admin_can_add_user(logged_in):
    r = logged_in.post(
        "/akun/users", data={"username": "sinta", "password": "rahasia2"}
    )
    assert r.status_code == 302
    assert find_user("sinta")["is_admin"] == 0


def test_admin_can_add_admin(logged_in):
    logged_in.post(
        "/akun/users",
        data={"username": "wawan", "password": "rahasia2", "is_admin": "1"},
    )
    assert find_user("wawan")["is_admin"] == 1


def test_invalid_username_rejected(logged_in):
    r = logged_in.post("/akun/users", data={"username": "a b", "password": "rahasia2"})
    assert r.status_code == 400
    assert find_user("a b") is None


def test_short_password_rejected(logged_in):
    r = logged_in.post("/akun/users", data={"username": "sinta", "password": "123"})
    assert r.status_code == 400
    assert find_user("sinta") is None


def test_duplicate_username_rejected(logged_in):
    logged_in.post("/akun/users", data={"username": "sinta", "password": "rahasia2"})
    r = logged_in.post("/akun/users", data={"username": "SINTA", "password": "rahasia3"})
    assert r.status_code == 400


def test_member_cannot_manage_users(member):
    r = member.post("/akun/users", data={"username": "sinta", "password": "rahasia2"})
    assert r.status_code == 403
    assert find_user("sinta") is None


def test_member_page_hides_user_list(member):
    r = member.get("/akun")
    assert r.status_code == 200
    assert "Daftar user" not in r.get_data(as_text=True)


def test_admin_resets_user_password(logged_in, client):
    logged_in.post("/akun/users", data={"username": "sinta", "password": "rahasia2"})
    user = find_user("sinta")
    r = logged_in.post(
        f"/akun/users/{user['id']}/password", data={"password": "baru123"}
    )
    assert r.status_code == 302
    client.post("/logout")
    assert client.post(
        "/login", data={"username": "sinta", "password": "baru123"}
    ).status_code == 302


def test_admin_cannot_delete_self(logged_in):
    admin = find_user("admin")
    r = logged_in.post(f"/akun/users/{admin['id']}/delete")
    assert r.status_code == 400
    assert find_user("admin") is not None


def test_admin_deletes_other_user(logged_in):
    logged_in.post("/akun/users", data={"username": "sinta", "password": "rahasia2"})
    user = find_user("sinta")
    r = logged_in.post(f"/akun/users/{user['id']}/delete")
    assert r.status_code == 302
    assert find_user("sinta") is None


def test_change_own_password_wrong_old(logged_in):
    r = logged_in.post(
        "/akun/password",
        data={"password_lama": "salah", "password_baru": "baru123"},
    )
    assert r.status_code == 400


def test_change_own_password(logged_in, client):
    r = logged_in.post(
        "/akun/password",
        data={"password_lama": "rahasia", "password_baru": "baru123"},
    )
    assert r.status_code == 302
    client.post("/logout")
    assert client.post(
        "/login", data={"username": "admin", "password": "baru123"}
    ).status_code == 302
