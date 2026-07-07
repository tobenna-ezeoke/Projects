import pytest
import sqlite3
import numpy as np
from unittest.mock import patch, MagicMock

# --- Pre-patch before import ---
with patch.dict('sys.modules', {
    'RPi': MagicMock(),
    'RPi.GPIO': MagicMock(),
    'picamera2': MagicMock(),
    'firebase_admin': MagicMock(),
    'firebase_admin.credentials': MagicMock(),
    'firebase_admin.db': MagicMock(),
    'websockets': MagicMock(),
}):
    import classifier_and_commander as commander

# --- Mocks for external services ---
@pytest.fixture(autouse=True)
def mock_gpio():
    with patch('classifier_and_commander.GPIO') as gpio_mock:
        gpio_mock.LOW = 0
        gpio_mock.input.return_value = gpio_mock.LOW
        yield gpio_mock

@pytest.fixture(autouse=True)
def mock_firebase():
    with patch('classifier_and_commander.firebase_ref') as firebase_mock:
        firebase_mock.push = MagicMock()
        yield firebase_mock

@pytest.fixture(autouse=True)
def mock_ws():
    with patch('classifier_and_commander.send_to_rpi2') as ws_mock:
        ws_mock.return_value = None
        yield ws_mock

# --- Tests ---
def test_classify_object_paper():
    print("📝 Testing classification: Paper")
    roi = (255 * np.ones((50, 50, 3), dtype=np.uint8))  # White patch simulates paper
    category, bin_id = commander.classify_object(roi)
    assert category == "Paper"
    assert bin_id == "1"
    print("✅ Passed Paper classification test.\n")

def test_classify_object_compost():
    print("🍃 Testing classification: Compost")
    roi = np.zeros((50, 50, 3), dtype=np.uint8)
    roi[:] = [0, 180, 0]  # Green patch simulates compost
    category, bin_id = commander.classify_object(roi)
    assert category == "Compost"
    assert bin_id == "4"
    print("✅ Passed Compost classification test.\n")

def test_classify_object_metal():
    print("🪙 Testing classification: Metal/Plastic")
    roi = np.full((50, 50, 3), 120, dtype=np.uint8)  # Grey tone for plastic/metal
    category, bin_id = commander.classify_object(roi)
    assert category == "Metal/Plastic"
    assert bin_id == "2"
    print("✅ Passed Metal/Plastic classification test.\n")

def test_classify_object_landfill():
    print("🗑️ Testing classification: Landfill")
    roi = np.full((50, 50, 3), 20, dtype=np.uint8)  # Dark object to trigger landfill
    category, bin_id = commander.classify_object(roi)
    assert category == "Landfill"
    assert bin_id == "3"
    print("✅ Passed Landfill classification test.\n")

def test_local_db_inserts(tmp_path):
    print("🗄️ Testing SQLite DB inserts")
    db_file = tmp_path / "test.db"
    conn = sqlite3.connect(db_file)
    c = conn.cursor()
    c.execute("CREATE TABLE metal_detection (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, detected INTEGER)")
    c.execute("CREATE TABLE presence_detection (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, present INTEGER)")
    c.execute("CREATE TABLE classifications (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, bin INTEGER, material TEXT)")
    c.execute("INSERT INTO metal_detection (detected) VALUES (1)")
    c.execute("INSERT INTO presence_detection (present) VALUES (1)")
    c.execute("INSERT INTO classifications (bin, material) VALUES (?, ?)", (1, "Paper"))
    conn.commit()
    assert c.execute("SELECT * FROM metal_detection").fetchall()
    assert c.execute("SELECT * FROM presence_detection").fetchall()
    assert c.execute("SELECT * FROM classifications").fetchall()
    conn.close()
    print("✅ Passed SQLite DB insertion test.\n")
