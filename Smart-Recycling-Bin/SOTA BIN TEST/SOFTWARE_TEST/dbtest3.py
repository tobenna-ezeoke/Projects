import sqlite3
import time
import firebase_admin
from firebase_admin import credentials, db

# Use your correct Firebase project URL
cred = credentials.Certificate("firebase_key.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://sotta-bin-default-rtdb.firebaseio.com'})

def test_rpi3_local_and_firebase():
    # --- Local DB Test ---
    conn = sqlite3.connect("rpi3_sensor_logs.db")
    c = conn.cursor()

    # Create table if it doesn't exist (to avoid OperationalError)
    c.execute('''CREATE TABLE IF NOT EXISTS classifications (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    bin INTEGER CHECK(bin BETWEEN 1 AND 4),
                    distance REAL CHECK(distance BETWEEN 0 AND 400)
                )''')

    c.execute("INSERT INTO classifications (bin, distance) VALUES (?, ?)", (1, 5))
    conn.commit()
    c.execute("SELECT * FROM classifications WHERE bin=1")
    assert len(c.fetchall()) > 0
    conn.close()
    print("[LOCAL TEST] Data successfully inserted and verified in rpi3_sensor_logs.db ✅")

    # --- Firebase Test ---
    ref = db.reference('/bins/bin1')
    ref.set({"status": "full", "last_updated": time.time()})
    snap = ref.get()
    assert snap["status"] == "full"
    print("[FIREBASE TEST] Firebase entry for bin1 verified as full ✅")

# Run it once to verify both local and Firebase integrity
if __name__ == "__main__":
    test_rpi3_local_and_firebase()
