"""
bin_full_monitor.py
-------------------
This script runs on a Raspberry Pi and monitors the fill status of paper and compost bins
using IR sensors connected to GPIO pins. It performs the following:

- Detects when a bin has reached a threshold fill level.
- Sends an alert via email using the Mailgun API when a bin is full.
- Displays an alert using the Sense HAT LED matrix.
- Waits for a reset signal from Firebase before resuming monitoring.
- Logs all "bin full" events to Firebase Realtime Database.

This system is part of the Smart Waste Sorting Bin project.
"""

import time
import firebase_admin
from firebase_admin import credentials, db
import RPi.GPIO as GPIO
import requests
from sense_hat import SenseHat

# --- Firebase Configuration ---
SERVICE_ACCOUNT_PATH = "firebase_key.json"
DATABASE_URL = ""

# --- IR Sensor Pin Configuration ---
IR_PINS = {
    4: "Paper bin",
    8: "Compost bin"
}
counter = {
    "Paper bin": 0,
    "Compost bin": 0
}

# --- Mailgun Configuration ---
API_KEY = ""
DOMAIN = ""
RECIPIENT_EMAIL = ""

# --- LED Display Colors ---
GREY = [128, 128, 128]   # Paper bin full
BROWN = [0, 0, 255]      # Compost bin full

# --- Setup Functions ---
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
    """Initialize Firebase using Admin SDK credentials."""
    if not firebase_admin._apps:
        cred = credentials.Certificate(SERVICE_ACCOUNT_PATH)
        firebase_admin.initialize_app(cred, {"databaseURL": DATABASE_URL})
        print("Firebase initialized.")

# --- Helper Functions ---
def get_timestamp():
    return time.strftime("%Y-%m-%d %H:%M:%S")

def send_bin_full_alert(bin_name):
    """Send an email notification when a bin is full."""
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
    """Wait for the 'reset' signal from Firebase and then clear the LED matrix."""
    print("Waiting for reset signal in Firebase...")
    reset_ref = db.reference("reset")

    while True:
        reset_val = reset_ref.get()
        if reset_val is not None:
            print("Reset signal received. Clearing LED and resuming.")
            sense.clear()
            reset_ref.delete()
            break
        time.sleep(2)

def check_sensors(sense):
    """Continuously monitor IR sensors for bin fullness."""
    global counter
    for pin, bin_name in IR_PINS.items():
        if GPIO.input(pin) == GPIO.LOW:
            print(f"Object detected in {bin_name}")
            counter[bin_name] += 1

            if counter[bin_name] >= 5:
                print(f"{bin_name} is full.")
                timestamp = get_timestamp()

                # Log to Firebase
                db.reference(f"{bin_name}/logs").push({
                    "status": "Bin Full",
                    "timestamp": timestamp
                })

                # Send alert
                send_bin_full_alert(bin_name)

                # Update LED matrix
                if bin_name == "Paper bin":
                    fill_matrix(sense, GREY)
                elif bin_name == "Compost bin":
                    fill_matrix(sense, BROWN)

                # Wait for Firebase reset
                wait_for_firebase_reset(sense)

                # Reset internal state
                counter[bin_name] = 0

                # Wait for physical item to be removed
                print(f"Waiting for {bin_name} to be cleared...")
                while GPIO.input(pin) == GPIO.LOW:
                    time.sleep(1)
                print(f"{bin_name} cleared. Monitoring resumed.")

# --- Main Execution ---
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
        print("Program stopped.")

if __name__ == "__main__":
    main()
