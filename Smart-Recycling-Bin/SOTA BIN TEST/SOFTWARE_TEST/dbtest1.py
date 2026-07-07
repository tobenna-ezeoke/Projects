import sqlite3
import time

def test_rpi1_db_integrity():
    conn = sqlite3.connect("rpi1_sensor_logs.db")
    c = conn.cursor()

    # Ensure tables exist before testing
    c.execute('''CREATE TABLE IF NOT EXISTS presence_detection (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    present INTEGER CHECK(present IN (0,1))
                )''')

    c.execute('''CREATE TABLE IF NOT EXISTS metal_detection (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    detected INTEGER CHECK(detected IN (0,1))
                )''')

    c.execute('''CREATE TABLE IF NOT EXISTS classifications (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    bin INTEGER CHECK(bin BETWEEN 1 AND 4),
                    material TEXT CHECK(material IN ('plastic', 'paper', 'metal', 'organic', 'other'))
                )''')

    conn.commit()

    print("[TEST] Inserting mock data...")
    # Insert mock data
    c.execute("INSERT INTO presence_detection (present) VALUES (1)")
    c.execute("INSERT INTO metal_detection (detected) VALUES (0)")
    c.execute("INSERT INTO classifications (bin, material) VALUES (?, ?)", (2, "metal"))
    conn.commit()

    # Verify data inserted
    print("[TEST] Verifying presence_detection table...")
    c.execute("SELECT * FROM presence_detection")
    rows = c.fetchall()
    print(f"[TEST] Retrieved {len(rows)} rows from presence_detection")
    assert len(rows) > 0

    c.execute("SELECT * FROM classifications WHERE material='metal'")
    rows = c.fetchall()
    print(f"[TEST] Retrieved {len(rows)} rows where material is metal")
    assert len(rows) == 1

    conn.close()
    print("[TEST] DB integrity check passed ✅")
