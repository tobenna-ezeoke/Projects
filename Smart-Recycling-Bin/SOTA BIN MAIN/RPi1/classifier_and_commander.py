"""
smart_bin_rpi1.py
-----------------
This script runs on Raspberry Pi 1 and is responsible for:
1. Capturing frames from PiCamera2 and detecting new objects via background subtraction.
2. Using sensor data (moisture + metal) to help classify waste materials.
3. Sending classified results to Firebase and RPi2 via WebSocket.
4. Logging classification events into a local SQLite database.
"""

# --- Imports ---
import time
import os
import json
import asyncio
import sqlite3
from datetime import datetime

import cv2
import numpy as np
import RPi.GPIO as GPIO
from picamera2 import Picamera2

import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

import firebase_admin
from firebase_admin import credentials, db
import websockets

# --- Constants ---
MOISTURE_THRESHOLD = 17800
BACKGROUND_FILE = "clean_bin_bg.npy"
WEBSOCKET_URI = "ws://172.17.182.70:8765"
DB_PATH = "/home/uchennaobikwelu/sysc3010-project-l2-g3/SOTA BIN MAIN/RPi1/sota_bin_log.db"
METAL_PIN = 4
CAMERA_RESOLUTION = (640, 480)
CLASSIFY_COOLDOWN = 6  # seconds

# --- GPIO + ADC Setup ---
GPIO.setmode(GPIO.BCM)
GPIO.setup(METAL_PIN, GPIO.IN)
i2c = busio.I2C(board.SCL, board.SDA)
ads = ADS.ADS1115(i2c)
chan0 = AnalogIn(ads, ADS.P0)
chan2 = AnalogIn(ads, ADS.P2)

# --- Firebase Setup ---
cred = credentials.Certificate("firebase_key.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://sotta-bin-default-rtdb.firebaseio.com'
})
firebase_ref = db.reference('/classification')

# --- SQLite Setup ---
conn = sqlite3.connect(DB_PATH)
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS presence_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    object_present INTEGER,
    timestamp TEXT
)''')
c.execute('''CREATE TABLE IF NOT EXISTS metal_sensor_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    metal_detected INTEGER,
    timestamp TEXT
)''')
c.execute('''CREATE TABLE IF NOT EXISTS moisture_levels (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sensor1_reading INTEGER,
    sensor2_reading INTEGER,
    timestamp TEXT
)''')
c.execute('''CREATE TABLE IF NOT EXISTS classified_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    bin_label TEXT,
    material TEXT,
    timestamp TEXT
)''')
conn.commit()

# --- Classification Logic ---
def is_moisture_high(r1, r2):
    return r1 < MOISTURE_THRESHOLD or r2 < MOISTURE_THRESHOLD

def classify_with_sensors(metal_present, moist, roi):
    if metal_present:
        return "Metal", "Plastic/Metal bin"
    elif moist:
        return "Compost", "Compost bin"
    else:
        return classify_fallback(roi)

def classify_fallback(roi):
    hsv = cv2.cvtColor(roi, cv2.COLOR_RGB2HSV)
    brightness = np.mean(roi)

    lower_paper = np.array([0, 0, 200])
    upper_paper = np.array([180, 35, 255])
    mask_paper = cv2.inRange(hsv, lower_paper, upper_paper)
    paper_area = cv2.countNonZero(mask_paper)

    gray = cv2.cvtColor(roi, cv2.COLOR_RGB2GRAY)
    highlights = cv2.threshold(gray, 230, 255, cv2.THRESH_BINARY)[1]
    highlight_pixels = cv2.countNonZero(highlights)
    edges = cv2.Canny(gray, 50, 150)
    edge_density = np.sum(edges) / 255

    print(f"[DEBUG] Brightness={brightness:.1f}, PaperArea={paper_area}, Highlights={highlight_pixels}, EdgeDensity={edge_density:.0f}")

    if highlight_pixels > 90 or (edge_density > 9500 and brightness > 110):
        return "Plastic", "Plastic/Metal bin"
    elif paper_area > 1000 and highlight_pixels < 70 and edge_density < 9000:
        return "Paper", "Paper bin"
    else:
        return "Landfill", "Landfill bin"

# --- WebSocket Sender ---
async def send_to_rpi2(category, bin_label):
    try:
        async with websockets.connect(WEBSOCKET_URI) as websocket:
            msg = json.dumps({"type": category, "bin": bin_label})
            await websocket.send(msg)
            print(f"[WebSocket] Sent: {msg}")
            response = await websocket.recv()
            print(f"[WebSocket] Received: {response}")
    except Exception as e:
        print(f"[WebSocket Error] {e}")

# --- Camera Setup ---
picam2 = Picamera2()
picam2.preview_configuration.main.size = CAMERA_RESOLUTION
picam2.preview_configuration.main.format = "RGB888"
picam2.preview_configuration.align()
picam2.configure("preview")
picam2.start()
time.sleep(2)

# --- Learn Background ---
def learn_background():
    bg_frame = picam2.capture_array()
    bg_gray = cv2.cvtColor(bg_frame, cv2.COLOR_RGB2GRAY)
    bg_gray = cv2.GaussianBlur(bg_gray, (21, 21), 0)
    np.save(BACKGROUND_FILE, bg_gray)
    print("[INFO] Background saved.")
    return bg_gray

# --- Main Loop ---
print("[INFO] Learning clean bin background...")
time.sleep(4)
bg_gray = learn_background()

cooldown = False
relearn_background_flag = False
last_class_time = 0
last_display_frame = None

print("[Smart Bin] Running. Press 'r' to relearn background. Press 'q' to quit.")

while True:
    frame = picam2.capture_array()
    gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
    gray = cv2.GaussianBlur(gray, (21, 21), 0)
    key = cv2.waitKey(1) & 0xFF

    if key == ord('r'):
        print("[INFO] Manual background relearn...")
        bg_gray = learn_background()

    if relearn_background_flag:
        print("[INFO] Auto-relearning background after classification...")
        bg_gray = learn_background()
        relearn_background_flag = False

    if cooldown and time.time() - last_class_time < CLASSIFY_COOLDOWN:
        frame_to_show = last_display_frame if last_display_frame is not None else frame
        cv2.imshow("Smart Bin", cv2.cvtColor(frame_to_show, cv2.COLOR_RGB2BGR))
        if key == ord('q'):
            break
        continue
    else:
        cooldown = False

    # Motion detection
    delta = cv2.absdiff(bg_gray, gray)
    thresh = cv2.threshold(delta, 25, 255, cv2.THRESH_BINARY)[1]
    thresh = cv2.dilate(thresh, None, iterations=3)
    contours, _ = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    detected = next((cnt for cnt in contours if cv2.contourArea(cnt) > 2000), None)

    if detected:
        (x, y, w, h) = cv2.boundingRect(detected)
        print("[INFO] New object detected. Waiting...")
        time.sleep(4)
        fresh_frame = picam2.capture_array()
        roi = fresh_frame[y:y+h, x:x+w]

        # Read sensors
        moisture1 = chan0.value
        moisture2 = chan2.value
        moist = is_moisture_high(moisture1, moisture2)
        metal_present = GPIO.input(METAL_PIN) == GPIO.LOW
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        category, bin_label = classify_with_sensors(metal_present, moist, roi)
        print(f"[INFO] Classified as: {category} --> {bin_label}")

        firebase_ref.push({
            "material": category,
            "bin": bin_label,
            "timestamp": timestamp
        })

        # Log to SQLite
        c.execute("INSERT INTO moisture_levels (sensor1_reading, sensor2_reading, timestamp) VALUES (?, ?, ?)",
                  (moisture1, moisture2, timestamp))
        c.execute("INSERT INTO metal_sensor_events (metal_detected, timestamp) VALUES (?, ?)",
                  (1 if metal_present else 0, timestamp))
        c.execute("INSERT INTO presence_events (object_present, timestamp) VALUES (?, ?)", (1, timestamp))
        c.execute("INSERT INTO classified_items (bin_label, material, timestamp) VALUES (?, ?, ?)",
                  (bin_label, category, timestamp))
        conn.commit()

        asyncio.run(send_to_rpi2(category, bin_label))

        cv2.rectangle(fresh_frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
        cv2.putText(fresh_frame, category, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX,
                    0.7, (0, 0, 255), 2)

        last_display_frame = fresh_frame.copy()
        last_class_time = time.time()
        cooldown = True
        relearn_background_flag = True

    cv2.imshow("Smart Bin", cv2.cvtColor(
        last_display_frame if last_display_frame is not None else frame, cv2.COLOR_RGB2BGR))

    if key == ord('q'):
        break

# --- Cleanup ---
picam2.stop()
GPIO.cleanup()
cv2.destroyAllWindows()
conn.close()
