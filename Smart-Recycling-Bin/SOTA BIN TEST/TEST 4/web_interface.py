from flask import Flask, render_template, request, jsonify
import firebase_admin
from firebase_admin import credentials, db
import time  # Add this at the top of your file

# Initialize Firebase
cred = credentials.Certificate("firebase_key.json")  # Use your actual Firebase credentials file
firebase_admin.initialize_app(cred, {'databaseURL': 'https://sotta-bin-default-rtdb.firebaseio.com'})

app = Flask(__name__)

# HTML Page
HTML_PAGE = """
<!DOCTYPE html>
<html>
<head>
    <title>Bin Management</title>
</head>
<body>
    <h2>Bin Reset Interface</h2>
    <button onclick="sendReset()">Reset Bin 3</button>
    <p id="status"></p>
    <script>
        function sendReset() {
            fetch('/reset', { method: 'POST' })
            .then(response => response.json())
            .then(data => document.getElementById("status").innerText = data.message);
        }
    </script>
</body>
</html>
"""

@app.route('/')
def home():
    return HTML_PAGE


@app.route('/reset', methods=['POST'])
def reset_bin():
    # Data to send to Firebase
    reset_action = {
        "action": "reset",
        "bin": "3",
        "timestamp": time.time()  # Ensure uniqueness
    }

    # Send reset command to Firebase (push creates a new entry every time)
    ref = db.reference("/manager_actions")
    ref.push(reset_action)  # Use push() instead of update()

    return jsonify({"message": "Reset action sent successfully!"})


if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, debug=True)
