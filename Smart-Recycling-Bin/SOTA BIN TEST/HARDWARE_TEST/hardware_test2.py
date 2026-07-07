import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.IN)  

try:
    while True:
        if GPIO.input(4) == GPIO.LOW:
            print("Metal Detected!")
        else:
            print("No Metal")
        time.sleep(0.5)
except KeyboardInterrupt:
    GPIO.cleanup()
