#!/bin/bash

echo "Starting Sota Bin WebSocket Servers and Clients..."

# Start RPi2 Server (Sorting Server)
echo "Starting RPi2 WebSocket Server..."
python3 rpi2_server.py &
RPi2_PID=$!
sleep 2  # Wait for server to initialize

# Start RPi1 Server (Bin Full Alert Server)
echo "Starting RPi1 WebSocket Server..."
python3 rpi1_server.py &
RPi1_PID=$!
sleep 2  # Wait for server to initialize

# Run Client on RPi1 to send classification to RPi2
echo "Sending Classification from RPi1 to RPi2..."
python3 rpi1_client.py

# Run Client on RPi3 to notify bin full (optional)
echo "Sending Bin Full Alert from RPi3 to RPi1..."
python3 rpi1_to_3_server.py

echo "All processes started successfully!"

# Wait for user to exit (optional)
wait $RPi2_PID
wait $RPi1_PID
