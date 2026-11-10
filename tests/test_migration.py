import sqlite3

from app import list_devices

OLD_SCHEMA = """
CREATE TABLE devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    token_hash TEXT NOT NULL UNIQUE,
    token TEXT,
    enabled INTEGER NOT NULL DEFAULT 1
)
"""


def test_old_schema_migrated_without_losing_devices(tmp_path, monkeypatch):
    db_file = tmp_path / "old.db"
    conn = sqlite3.connect(db_file)
    conn.execute(OLD_SCHEMA)
    conn.execute(
        "INSERT INTO devices (name, token_hash) VALUES ('led-lama', 'hash-lama')"
    )
    conn.commit()
    conn.close()

    monkeypatch.setenv("DB_PATH", str(db_file))

    devices = list_devices()
    assert len(devices) == 1
    assert devices[0]["name"] == "led-lama"
    assert devices[0]["type"] == "led"
    assert devices[0]["temperature"] is None
    assert devices[0]["led_on"] == 0
