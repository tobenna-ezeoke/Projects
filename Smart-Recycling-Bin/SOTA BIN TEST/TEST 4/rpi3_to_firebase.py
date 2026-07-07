import firebase_admin
from firebase_admin import credentials, db
import json
import time

# Initialize Firebase
cred = credentials.Certificate("firebase_key.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://sotta-bin-default-rtdb.firebaseio.com'})

# Bin full status data with timestamp
bin_status = {
    "bin": "3",
    "status": "Full",
    "timestamp": time.time()  # Adds a timestamp for uniqueness
}

# Send status to Firebase
ref = db.reference("/bin_status")
ref.push(bin_status)  # Use push() to create a new entry

print("Bin full status sent:", json.dumps(bin_status, indent=2))
