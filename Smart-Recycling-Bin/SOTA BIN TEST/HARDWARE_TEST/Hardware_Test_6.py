import RPi.GPIO as GPIO
import time

# **Stepper Motor Configuration (ULN2003 Driver)**
STEPPER_PINS = [14, 15, 18, 23]  # GPIOs connected to IN1-IN4

# **Step sequence for 4-step stepping (28BYJ-48 Stepper Motor)**
STEP_SEQUENCE = [
    [1, 0, 0, 1],
    [1, 1, 0, 0],
    [0, 1, 1, 0],
    [0, 0, 1, 1]
]

# **Bin Positions (Degrees)**
BIN_POSITIONS = [0, 90, 180, 270]  # Bins 1 to 4

# **Initialize Raspberry Pi GPIO for Stepper Motor**
GPIO.setmode(GPIO.BCM)
for pin in STEPPER_PINS:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, 0)

def rotate_stepper(target_angle, current_angle, delay=0.002):
    """ Rotate stepper motor to desired bin position """
    steps_per_rev = 2048  # Adjust based on stepper motor specs (28BYJ-48 has ~2048 steps/rev)
    step_diff = target_angle - current_angle

    # Determine direction and number of steps
    steps = int((step_diff / 360) * steps_per_rev)
    step_sequence = STEP_SEQUENCE if steps > 0 else list(reversed(STEP_SEQUENCE))

    for _ in range(abs(steps)):
        for step in step_sequence:
            for pin in range(4):
                GPIO.output(STEPPER_PINS[pin], step[pin])
            time.sleep(delay)
    
    return target_angle  # Update the current angle

try:
    current_position = 0  # Start at home position
    while True:
        for target_position in BIN_POSITIONS:
            print(f"Moving Stepper Motor to {target_position}°...")
            current_position = rotate_stepper(target_position, current_position)
            time.sleep(2)  # Pause before moving to the next bin

except KeyboardInterrupt:
    print("Resetting system to original state...")
    GPIO.cleanup()
    print("Test stopped.")
