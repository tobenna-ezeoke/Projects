import firebase_admin
from firebase_admin import credentials, db
import json
import time

# Initialize Firebase
cred = credentials.Certificate("firebase_key.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://sotta-bin-default-rtdb.firebaseio.com'})

# Generate classification data with timestamp
classification_data = {
    "item": "plastic",
    "bin": "2",
    "timestamp": time.time()  # Adds a timestamp for uniqueness
}

# Send data to Firebase
ref = db.reference("/classification_data")
ref.push(classification_data)  # Use push() to add a new entry instead of replacing

print("Classification data sent:", json.dumps(classification_data, indent=2))
