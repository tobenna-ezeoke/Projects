#!/bin/bash

echo "Starting Sota Bin End-to-End Automation..."

# Define the Python path in the virtual environment
MY_PYTHON="/home/uchennaobikwelu/sysc3010-labs-uchennaobi/sysc3010-project-l2-g3/SOTA BIN/TEST 5/myenv5/bin/python3"

echo "Using Python: $MY_PYTHON"

# Check for existing Flask process and kill it
echo "Checking for existing Flask process on port 5000..."
sudo kill -9 $(sudo lsof -t -i:5000) 2>/dev/null || echo "No existing process found."

# Start RPi1 -> Firebase (Classification Data)
echo "Starting rpi1_to_firebase.py..."
"$MY_PYTHON" rpi1_to_firebase.py &
RPi1_PID=$!
echo "Started rpi1_to_firebase.py with PID $RPi1_PID"
sleep 2  

# Keep the script running and wait for processes
wait $RPi1_PID

