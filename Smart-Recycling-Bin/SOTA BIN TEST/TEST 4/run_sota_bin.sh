#!/bin/bash

echo "Starting Sota Bin End-to-End Automation..."

# Define the Python path in the virtual environment
MY_PYTHON="/home/uchennaobikwelu/sysc3010-labs-uchennaobi/sysc3010-project-l2-g3/SOTA BIN/TEST 5/myenv5/bin/python3"

echo "Using Python: $MY_PYTHON"

# Check for existing Flask process and kill it
echo "Checking for existing Flask process on port 5000..."
sudo kill -9 $(sudo lsof -t -i:5000) 2>/dev/null || echo "No existing process found."

# Start Web Interface (Flask Server)
echo "Starting web_interface.py..."
"$MY_PYTHON" web_interface.py &
WEB_PID=$!
echo "Started web_interface.py with PID $WEB_PID"
sleep 5  

# Open the Web Interface in Browser
echo "Opening Web Interface in Browser..."
xdg-open http://127.0.0.1:5000 &  

# Start RPi3 -> Firebase (Bin Full Alert)
echo "Starting rpi3_to_firebase.py..."
"$MY_PYTHON" rpi3_to_firebase.py &
RPi3_PID=$!
echo "Started rpi3_to_firebase.py with PID $RPi3_PID"
sleep 2  

# Start Firebase Listener for Reset (RPi3)
echo "Starting firebase_to_rpi3.py..."
"$MY_PYTHON" firebase_to_rpi3.py &
Firebase_Listener_PID=$!
echo "Started firebase_to_rpi3.py with PID $Firebase_Listener_PID"
sleep 2  

# Simulate Reset Action via Web Interface
echo "Simulating Reset Action from Web Interface..."
curl -X POST http://127.0.0.1:5000/reset
echo "Reset Action Sent."

# Keep the script running and wait for processes
wait $WEB_PID
wait $RPi3_PID
wait $Firebase_Listener_PID
