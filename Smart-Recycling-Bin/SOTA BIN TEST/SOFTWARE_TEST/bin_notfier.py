import firebase_admin
from firebase_admin import credentials, db
from sense_hat import SenseHat
import time
import requests

# --- Firebase Setup ---
cred = credentials.Certificate("firebase_key.json")
firebase_admin.initialize_app(cred, {'databaseURL': ''})

# --- Sense HAT ---
sense = SenseHat()

# --- Mailgun Alert ---
def send_bin_full_alert(bin_name):
    api_key = ""
    domain = ""
    return requests.post(
        f"https://api.mailgun.net/v3/{domain}/messages",
        auth=("api", api_key),
        data={
            "from": f"Smart Bin <postmaster@{domain}>",
            "to": "binmanager8@gmail.com",
            "subject": f"⚠️ ALERT: {bin_name} Bin Full",
            "text": f"The {bin_name} bin is full. Please empty it ASAP."
        })

def main():
    
    # Assume bins are always full, so we skip the checks and just send alerts
    sense.show_message("Landfill bin full", text_colour=[0, 255, 0])
    send_bin_full_alert("Landfill")

    sense.show_message("Plastic/metal bin full", text_colour=[255, 255, 255])
    send_bin_full_alert("Plastic/metal")

    time.sleep(5)

if __name__ == "__main__":
    main()

