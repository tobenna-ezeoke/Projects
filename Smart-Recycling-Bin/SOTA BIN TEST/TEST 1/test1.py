#!/usr/bin/python3
import random
import time
from signal import signal, SIGTERM, SIGHUP, pause
from rpi_lcd import LCD

lcd = LCD()

def safe_exit(signum, frame):
    lcd.clear()
    exit(1)

signal(SIGTERM, safe_exit)
signal(SIGHUP, safe_exit)

def simulate_camera():
    return "image_001.jpg"  # Simulated image file name

def simulate_color_sensor():
    return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

def simulate_metal_sensor():
    return random.choice([0, 1])  # 1 = Metal detected, 0 = No metal

def simulate_ir_sensor():
    return random.choice([0, 1])  # 1 = Object present, 0 = No object

try:
    while True:
        image = simulate_camera()
        color = simulate_color_sensor()
        metal = simulate_metal_sensor()
        presence = simulate_ir_sensor()

        # Printing simulated values (For debugging)
        print(f"Image Captured: {image}")
        print(f"Color Sensor: {color}")
        print(f"Metal Detected: {'Yes' if metal else 'No'}")
        print(f"Object Present: {'Yes' if presence else 'No'}")

        # Updating LCD Display
        lcd.text(f"Metal: {'Yes' if metal else 'No'}", 1)
        lcd.text(f"Object: {'Yes' if presence else 'No'}", 2)
        lcd.text("Sorting", 3)
        time.sleep(2)  # Delay before next simulation cycle

except KeyboardInterrupt:
    pass

finally:
    lcd.clear()
