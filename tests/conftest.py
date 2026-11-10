import os

import pytest
from werkzeug.security import generate_password_hash

os.environ["SECRET_KEY"] = "test-secret"
os.environ["ADMIN_USER"] = "admin"
os.environ["ADMIN_PASSWORD_HASH"] = generate_password_hash("rahasia")
os.environ["API_TOKEN"] = "token-manusia"

from app import STATE, add_device, create_app  # noqa: E402


@pytest.fixture(autouse=True)
def reset_state(tmp_path, monkeypatch):
    monkeypatch.setenv("DB_PATH", str(tmp_path / "test.db"))
    STATE["on"] = False


@pytest.fixture
def client():
    app = create_app()
    app.config.update(TESTING=True)
    return app.test_client()


@pytest.fixture
def logged_in(client):
    client.post("/login", data={"username": "admin", "password": "rahasia"})
    return client


@pytest.fixture
def device_token(reset_state):
    return add_device("led-1")
