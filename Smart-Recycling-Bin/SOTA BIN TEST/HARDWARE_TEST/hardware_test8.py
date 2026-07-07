import sqlite3
import RPi.GPIO as GPIO
import time
from datetime import datetime

# GPIO Pin Definitions
TRIG1, ECHO1 = 23, 24  # Sensor 1
TRIG2, ECHO2 = 20, 21  # Sensor 2

def initialize_db():
    """Initialize the SQLite database for storing sensor logs."""
    db_path = "rpi3_sensor_logs.db"
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS classifications (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            bin INTEGER CHECK(bin BETWEEN 1 AND 4),
            distance REAL CHECK(distance BETWEEN 0 AND 400)
        )
    ''')
    conn.commit()
    conn.close()

def insert_distance(bin_number, distance):
    """Insert sensor distance readings into the database."""
    db_path = "rpi3_sensor_logs.db"
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("INSERT INTO classifications (bin, distance) VALUES (?, ?)", (bin_number, distance))
    conn.commit()
    conn.close()

def retrieve_last_distances():
    """Retrieve the last 5 sensor readings."""
    db_path = "rpi3_sensor_logs.db"
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM classifications ORDER BY timestamp DESC LIMIT 5")
    records = cursor.fetchall()
    conn.close()
    return records

def setup_gpio():
    """Setup GPIO pins for ultrasonic sensors."""
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(TRIG1, GPIO.OUT)
    GPIO.setup(ECHO1, GPIO.IN)
    GPIO.setup(TRIG2, GPIO.OUT)
    GPIO.setup(ECHO2, GPIO.IN)

def measure_distance(TRIG, ECHO):
    """Measure distance using ultrasonic sensor."""
    GPIO.output(TRIG, True)
    time.sleep(0.00001)  # 10us pulse
    GPIO.output(TRIG, False)
    
    start, end = 0, 0
    while GPIO.input(ECHO) == 0:
        start = time.time()
    while GPIO.input(ECHO) == 1:
        end = time.time()
    
    distance = (end - start) * 34300 / 2  # Speed of sound: 34300 cm/s
    return round(distance, 2)

def check_bin_status(distance, sensor_id, full_count):
    """Check if the bin is full based on sensor readings."""
    if distance > 750 or distance < 3.5:
        full_count[sensor_id] += 1
        if full_count[sensor_id] >= 5:
            print(f"Sensor {sensor_id} confirms: BIN IS FULL")
        else:
            print(f"Sensor {sensor_id}: Bin might be full ({full_count[sensor_id]}/5 detections)")
    else:
        full_count[sensor_id] = 0  # Reset counter if normal

def main():
    """Main function to continuously monitor bin status."""
    initialize_db()
    setup_gpio()
    full_count = {1: 0, 2: 0}
    
    try:
        while True:
            dist1 = measure_distance(TRIG1, ECHO1)
            dist2 = measure_distance(TRIG2, ECHO2)
            print(f"Sensor 1 Distance, Bin 1: {dist1} cm")
            check_bin_status(dist1, 1, full_count)
            insert_distance(1, dist1)
            
            print(f"Sensor 2 Distance, Bin 2: {dist2} cm")
            check_bin_status(dist2, 2, full_count)
            insert_distance(2, dist2)
            
            time.sleep(1)
    except KeyboardInterrupt:
        print("Measurement stopped by user")
        GPIO.cleanup()

if __name__ == "__main__":
    main()
