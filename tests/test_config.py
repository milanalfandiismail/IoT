import pytest

from app import create_app


def test_empty_api_token_rejected(monkeypatch):
    monkeypatch.setenv("API_TOKEN", "  ")
    with pytest.raises(RuntimeError, match="API_TOKEN"):
        create_app()


def test_identical_tokens_rejected(monkeypatch):
    monkeypatch.setenv("DEVICE_TOKEN", "token-manusia")
    with pytest.raises(RuntimeError, match="API_TOKEN"):
        create_app()
