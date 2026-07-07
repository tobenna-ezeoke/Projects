import time
from sense_hat import SenseHat
import firebase_admin
from firebase_admin import credentials, db

# Initialize Firebase
cred = credentials.Certificate("firebase_key.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://sotta-bin-default-rtdb.firebaseio.com'})

# Initialize Sense HAT
sense = SenseHat()

def reset_listener(event):
    """Callback function when a new reset action is added to Firebase."""
    if event.data and isinstance(event.data, dict) and event.data.get("action") == "reset":
        print("Received Reset Action:", event.data)

        # Turn Sense HAT Red
        sense.clear((255, 0, 0))
        time.sleep(2)  # Simulated Reset Duration

        # Clear Sense HAT
        sense.clear()
        print("Reset complete, Sense HAT cleared.")

# Listen for reset actions in Firebase
ref = db.reference("/manager_actions")
ref.listen(reset_listener)
