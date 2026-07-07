"""
bin_full_monitor.py
-------------------
This script runs on Raspberry Pi 4 and monitors the fill level of the Plastic and Landfill bins
using IR sensors connected to GPIO pins.

Features:
- Detects when either bin reaches a predefined threshold (5 detections).
- Sends an email alert using the Mailgun API when a bin is full.
- Displays a color-coded warning using the Sense HAT LED matrix.
- Waits for a reset command via Firebase before resuming monitoring.
- Logs all full-bin events to Firebase Realtime Database.

This script is part of the Smart Waste Sorting Bin project.
"""

import time
import firebase_admin
from firebase_admin import credentials, db
import RPi.GPIO as GPIO
import requests
from sense_hat import SenseHat

# --- Firebase Admin SDK Configuration ---
SERVICE_ACCOUNT_PATH = "firebase_key.json"
DATABASE_URL = "https://sotta-bin-default-rtdb.firebaseio.com/"

# --- IR Sensor Configuration ---
IR_PINS = {
    4: "Plastic bin",
    8: "Landfill bin"
}
counter = {
    "Plastic bin": 0,
    "Landfill bin": 0
}

# --- Mailgun API Configuration ---
API_KEY = ""
DOMAIN = ""
RECIPIENT_EMAIL = "binmanager8@gmail.com"

# --- LED Display Colors ---
GREY = [128, 128, 128]   # Plastic bin full
BROWN = [0, 0, 255]      # Landfill bin full

# --- Utility Functions ---
def fill_matrix(sense, color):
    """Fill the Sense HAT LED matrix with the given color."""
    sense.clear()
    sense.set_pixels([color] * 64)

def initialize_gpio():
    """Set up GPIO pins for IR sensors."""
    GPIO.setmode(GPIO.BCM)
    for pin in IR_PINS:
        GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def initialize_firebase():
    """Initialize Firebase connection."""
    if not firebase_admin._apps:
        cred = credentials.Certificate(SERVICE_ACCOUNT_PATH)
        firebase_admin.initialize_app(cred, {"databaseURL": DATABASE_URL})
        print("Firebase initialized successfully.")

def get_timestamp():
    """Return the current timestamp."""
    return time.strftime("%Y-%m-%d %H:%M:%S")

def send_bin_full_alert(bin_name):
    """Send an email alert using Mailgun when a bin is full."""
    response = requests.post(
        f"https://api.mailgun.net/v3/{DOMAIN}/messages",
        auth=("api", API_KEY),
        data={
            "from": f"Smart Bin <postmaster@{DOMAIN}>",
            "to": RECIPIENT_EMAIL,
            "subject": f"High Priority: {bin_name} Bin Full",
            "text": f"The {bin_name} bin is full. Please empty it as soon as possible.",
        })

    if response.status_code == 200:
        print(f"Alert sent: {bin_name} bin is full.")
    else:
        print(f"Failed to send alert. Status code: {response.status_code} - {response.text}")

def wait_for_firebase_reset(sense):
    """Pause until 'reset' key is found in Firebase."""
    print("Waiting for reset signal in Firebase...")
    reset_ref = db.reference("reset")

    while True:
        reset_val = reset_ref.get()
        if reset_val is not None:
            print("Reset signal received. Clearing LED matrix.")
            sense.clear()
            reset_ref.delete()
            break
        time.sleep(2)

def check_sensors(sense):
    """Monitor each bin’s IR sensor for object detection."""
    global counter
    for pin, bin_name in IR_PINS.items():
        if GPIO.input(pin) == GPIO.LOW:
            print(f"Object detected in {bin_name}")
            counter[bin_name] += 1

            if counter[bin_name] >= 5:
                print(f"{bin_name} is full.")
                timestamp = get_timestamp()

                # Push event to Firebase
                db.reference(f"{bin_name}/logs").push({
                    "status": "Bin Full",
                    "timestamp": timestamp
                })

                # Send alert
                send_bin_full_alert(bin_name)

                # Show LED matrix warning
                if bin_name == "Plastic bin":
                    fill_matrix(sense, GREY)
                elif bin_name == "Landfill bin":
                    fill_matrix(sense, BROWN)

                # Wait for Firebase reset
                wait_for_firebase_reset(sense)

                # Reset counter
                counter[bin_name] = 0

                # Wait for item to be physically cleared
                print(f"Waiting for {bin_name} to be cleared...")
                while GPIO.input(pin) == GPIO.LOW:
                    time.sleep(1)
                print(f"{bin_name} cleared. Monitoring resumed.")

# --- Main Function ---
def main():
    initialize_firebase()
    initialize_gpio()
    sense = SenseHat()
    sense.clear()

    try:
        while True:
            check_sensors(sense)
            time.sleep(1)
    except KeyboardInterrupt:
        sense.clear()
        GPIO.cleanup()
        print("Program stopped by user.")

if __name__ == "__main__":
    main()
