
# RPi2 - Motor Control Server

This directory contains the WebSocket-based motor control system for the Smart Waste Sorting Bin. The script `final_motor.py` is designed to run on **Raspberry Pi 2 (RPi2)** and acts as a receiver for classification results sent from RPi1. Based on the received data, it controls:

- A stepper motor to rotate a platform or sorting mechanism.
- Two servo motors to drop the waste item into the correct bin.


## File

- `final_motor.py`: Sets up a WebSocket server on RPi2 that listens for classification results from RPi1, processes them, and activates motors to physically sort the waste.


## Functionality

When a connection is established and a classification message is received in JSON format, the system:

1. Parses the `type` (material) and `bin` label.
2. Uses the bin label to determine the target angle for the stepper motor.
3. Rotates the stepper motor to the corresponding bin position (if not already there).
4. Activates the servos to drop the item.
5. Returns the stepper motor to the default "home" position.

Example input:
```json
{
  "type": "Plastic",
  "bin": "Plastic/Metal bin"
}
```

## Bin Angle Configuration

The file contains a `bin_angles` dictionary that maps bin labels to rotation angles in degrees:

```python
bin_angles = {
    "Plastic/Metal bin": 0,
    "Landfill bin": 90,
    "Compost bin": 200,
    "Paper bin": -90
}
```

Adjust the angles as needed to match your mechanical layout.


## Requirements

Install the following packages:

```bash
pip install websockets pigpio RPi.GPIO
```

Hardware dependencies:
- 1 stepper motor connected to GPIO pins (14, 15, 18, 23)
- 2 servo motors connected to GPIO pins (20 and 21)
- External power supply for motors recommended

Ensure the `pigpiod` daemon is running:
```bash
sudo pigpiod
```

---

## How to Run

Navigate to the `RPi2/` directory and execute:

```bash
python3 final_motor.py
```

This starts a WebSocket server on:
```
ws://0.0.0.0:8765
```

It will continuously listen for incoming classification data from RPi1.


## Notes

- Stepper motor uses an 8-step sequence (half-stepping) for smooth rotation.
- Servo control is implemented using the `pigpio` library.
- The system is designed to respond to a single classification event per connection. Future improvements could include persistent connections and batched commands.


## Future Improvements

- Add end-stop sensors to detect actual home position of the stepper motor.
- Implement timeout handling in case no message is received.
- Add WebSocket authentication for more secure communication.
- Use object pooling or message queues to handle concurrent classification events.
- Add buzzer or LED indicators for system status (e.g., “Motor moving”, “Idle”).


## Author

Tobiloba Ola  
Carleton University  
SYSC 3010A – Computer Systems Development Project  
Winter 2025

