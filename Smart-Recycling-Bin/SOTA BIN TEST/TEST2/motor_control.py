import RPi.GPIO as GPIO
import time
from gpiozero import AngularServo
from gpiozero.pins.pigpio import PiGPIOFactory
from gpiozero import OutputDevice
from time import sleep

# Pin Definitions
IN1 = OutputDevice(14)
IN2 = OutputDevice(15)
IN3 = OutputDevice(18)
IN4 = OutputDevice(23)

# Define step sequence for the motor
step_sequence = [
    [1, 0, 0, 0],
    [1, 1, 0, 0],
    [0, 1, 0, 0],
    [0, 1, 1, 0],
    [0, 0, 1, 0],
    [0, 0, 1, 1],
    [0, 0, 0, 1],
    [1, 0, 0, 1]
]

def set_step(w1, w2, w3, w4):
    IN1.value = w1
    IN2.value = w2
    IN3.value = w3
    IN4.value = w4

def step_motor(steps, direction=1, delay=0.01):
    for _ in range(steps):
        for step in (step_sequence if direction > 0 else reversed(step_sequence)):
            set_step(*step)
            sleep(delay)

# Setup Servo Motor on GPIO 24
factory = PiGPIOFactory()
servo = AngularServo(24, min_angle=0, max_angle=180, pin_factory=factory)

def move_servo():
    """Moves the servo motor from 0° to 180° and back, then stops."""
    print("Moving Servo Motor...")
    for angle in range(0, 181, 45):  # Move in 45-degree steps
        servo.angle = angle
        print(f" Servo moved to {angle}°")
        time.sleep(1)

    for angle in range(180, -1, -45):
        servo.angle = angle
        print(f" Servo moved to {angle}°")
        time.sleep(1)

    # Stop the servo by setting it to None
    servo.angle = None
    print("Servo movement complete. Servo is now stopped.")

def move_stepper(steps=200, delay=0.002, direction="cw"):
    """Moves the stepper motor in specified direction for given steps."""
    print(f"Moving Stepper Motor {steps} steps ({direction.upper()})...")

    # Call the step_motor function to move the motor
    step_motor(steps, direction=1 if direction == "cw" else -1, delay=delay)

    print(f" Stepper motor finished {steps} steps ({direction.upper()}).")

try:
    while True:
        move_servo()
        move_stepper(steps=200, direction="cw")  # Rotate clockwise
        time.sleep(2)
        move_stepper(steps=200, direction="ccw")  # Rotate counterclockwise
        time.sleep(2)

except KeyboardInterrupt:
    print("Stopping motors and cleaning up GPIO...")
    servo.angle = None  # Ensure the servo stops before exiting
    GPIO.cleanup()
