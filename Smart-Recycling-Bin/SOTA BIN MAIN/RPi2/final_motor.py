"""
final_motor.py
--------------
This script runs on Raspberry Pi 2 and controls the mechanical actuation system
for the Smart Waste Sorting Bin.

It performs the following tasks:
- Sets up a WebSocket server that listens for classification data sent by RPi1.
- Moves a stepper motor to position the bin platform based on classification.
- Uses two servo motors to release the waste item into the correct bin.
- Returns the stepper motor to the home position after sorting.

Incoming JSON format:
{
    "type": "Plastic",
    "bin": "Plastic/Metal bin"
}

WebSocket Server: ws://0.0.0.0:8765
"""

import asyncio
import websockets
import json
import pigpio
import time
import RPi.GPIO as GPIO

# --- Constants & Configuration ---
IN1, IN2, IN3, IN4 = 14, 15, 18, 23  # Stepper motor GPIO pins
SERVO_1, SERVO_2 = 20, 21           # Servo GPIO pins
STEPS_PER_REV = 4096               # Full 360° rotation
DEGREE_TO_STEP = STEPS_PER_REV / 360
STEP_SEQUENCE = [
    [1, 0, 0, 0],
    [1, 1, 0, 0],
    [0, 1, 0, 0],
    [0, 1, 1, 0],
    [0, 0, 1, 0],
    [0, 0, 1, 1],
    [0, 0, 0, 1],
    [1, 0, 0, 1]
]

# Bin position angles in degrees
BIN_ANGLES = {
    "Plastic/Metal bin": 0,
    "Landfill bin": 90,
    "Compost bin": 200,
    "Paper bin": -90
}

# --- Initialization ---
GPIO.setmode(GPIO.BCM)
GPIO.setup([IN1, IN2, IN3, IN4], GPIO.OUT)
current_position = 0  # Tracks stepper motor position
pi = pigpio.pi()      # Initialize pigpio daemon for servos


# --- Stepper Motor Functions ---
def set_step(pins):
    """Activate stepper motor coils."""
    GPIO.output(IN1, pins[0])
    GPIO.output(IN2, pins[1])
    GPIO.output(IN3, pins[2])
    GPIO.output(IN4, pins[3])

def move_to_target(degrees, delay=0.002):
    """Move stepper motor to the specified degree position."""
    global current_position
    target_position = int(degrees * DEGREE_TO_STEP)
    step_count = len(STEP_SEQUENCE)

    while current_position != target_position:
        direction = 1 if target_position > current_position else -1
        step_index = (current_position + direction) % step_count
        set_step(STEP_SEQUENCE[step_index])
        time.sleep(delay)
        current_position += direction

    set_step([0, 0, 0, 0])  # Stop motor

def go_home():
    """Reset stepper motor to the home position (0°)."""
    global current_position
    move_to_target(0)
    current_position = 0
    print("Stepper motor returned to home position.")


# --- Servo Functions ---
def set_angle(servo1, angle1, servo2, angle2):
    """Move two servo motors to specified angles."""
    pulse_width_1 = int((angle1 / 180.0) * 2000 + 500)
    pulse_width_2 = int((angle2 / 180.0) * 2000 + 500)
    pi.set_servo_pulsewidth(servo1, pulse_width_1)
    pi.set_servo_pulsewidth(servo2, pulse_width_2)
    time.sleep(0.02)


# --- Main Logic ---
def process_waste(category, bin_label):
    """Execute mechanical sorting based on the received classification."""
    target_angle = BIN_ANGLES.get(bin_label, 0)

    # Rotate platform if necessary
    if target_angle != 0:
        move_to_target(target_angle)

    # Drop item using servo motion
    print("Activating servos to release item.")
    set_angle(SERVO_1, 90, SERVO_2, 0)
    time.sleep(2)

    print("Dropping item...")
    set_angle(SERVO_1, 0, SERVO_2, 90)
    time.sleep(2)

    print("Resetting servo arms...")
    set_angle(SERVO_1, 90, SERVO_2, 0)
    time.sleep(1)

    # Return platform to home
    if target_angle != 0:
        print("Returning stepper motor to home position.")
        go_home()


# --- WebSocket Server ---
async def websocket_server(websocket):
    """Handle incoming WebSocket connection and classification data."""
    try:
        data = await websocket.recv()
        print("Received data:", data)

        try:
            data = json.loads(data)
        except json.JSONDecodeError:
            print("Invalid JSON received.")
            await websocket.send(json.dumps({"error": "Invalid JSON format"}))
            return

        category = data.get("type")
        bin_label = data.get("bin")

        if category and bin_label:
            process_waste(category, bin_label)
            await websocket.send(json.dumps({"status": "received"}))
        else:
            await websocket.send(json.dumps({"error": "Missing data fields"}))

    except Exception as e:
        print(f"WebSocket error: {e}")


async def main():
    """Start the WebSocket server on port 8765."""
    async with websockets.serve(websocket_server, "0.0.0.0", 8765):
        print("WebSocket server started on ws://0.0.0.0:8765")
        await asyncio.Future()  # Keep server running


# --- Entry Point ---
if __name__ == "__main__":
    asyncio.run(main())
