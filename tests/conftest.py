import os

import pytest

os.environ["SECRET_KEY"] = "test-secret"
os.environ["API_TOKEN"] = "token-manusia"

from app import add_device, add_user, create_app  # noqa: E402


@pytest.fixture(autouse=True)
def reset_state(tmp_path, monkeypatch):
    monkeypatch.setenv("DB_PATH", str(tmp_path / "test.db"))


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
def member(client):
    add_user("budi", "rahasia1", False)
    client.post("/login", data={"username": "budi", "password": "rahasia1"})
    return client


@pytest.fixture
def led(reset_state):
    return add_device("led-1", "led")


@pytest.fixture
def suhu(reset_state):
    return add_device("suhu-1", "suhu")
