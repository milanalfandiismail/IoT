import pytest

from app import create_app


def test_empty_api_token_rejected(monkeypatch):
    monkeypatch.setenv("API_TOKEN", "  ")
    with pytest.raises(RuntimeError, match="API_TOKEN"):
        create_app()


def test_empty_secret_key_rejected(monkeypatch):
    monkeypatch.setenv("SECRET_KEY", "")
    with pytest.raises(RuntimeError, match="SECRET_KEY"):
        create_app()
