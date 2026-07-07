import pigpio
import time

# **Servo Motor Configuration**
SERVO_1 = 14 # GPIO pin for Servo 1 (Normal Rotation)
SERVO_2 = 15 # GPIO pin for Servo 2 (Opposite Rotation)

# **Initialize pigpio for Servo Motors**
pi = pigpio.pi()

def set_angle(servo, angle):
    """ Convert angle to PWM pulse width and set servo position """
    pulse_width = int((angle / 180.0) * 2000 + 500)  # Convert angle to PWM pulse width
    pi.set_servo_pulsewidth(servo, pulse_width)

try:
    while True:
        print("Servo 1 (GPIO25) to 90°, Servo 2 (GPIO24) to 90° (Opposite)...")
        set_angle(SERVO_1, 90)
        set_angle(SERVO_2, 90)
        time.sleep(2)

        print("Servo 1 to 0°, Servo 2 to 180° (Item Drop)...")
        set_angle(SERVO_1, 0)
        set_angle(SERVO_2, 180)
        time.sleep(2)
        
        print("Servo 1 (GPIO25) to 90°, Servo 2 (GPIO24) to 90° (Opposite)...")
        set_angle(SERVO_1, 90)
        set_angle(SERVO_2, 90)
        time.sleep(1)

except KeyboardInterrupt:
    print("Resetting system to original state...")
    set_angle(SERVO_1, 90)
    set_angle(SERVO_2, 90)
    time.sleep(1)

    pi.set_servo_pulsewidth(SERVO_1, 0)
    pi.set_servo_pulsewidth(SERVO_2, 0)
    pi.stop()
    print("Test stopped.")
