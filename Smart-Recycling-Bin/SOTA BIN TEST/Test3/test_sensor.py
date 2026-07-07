import time
import random
from rpi_lcd import LCD

# Initialize LCD
lcd = LCD()

def get_random_distance_mm():
    # Simulate random distance between 0 and 2000 mm (0-200 cm)
    return round(random.uniform(0, 400), 2)

def display_distance(distance):
    # Check distance and display accordingly
    if distance == 0:
        lcd.text("Empty", 1)
    elif distance < 200:
        lcd.text("Half", 1)
    else:
        lcd.text("Full", 1)

try:
    while True:
        # Get random simulated distance in millimeters
        distance = get_random_distance_mm()
        print(f"Distance: {distance} mm")

        # Display on LCD screen
        display_distance(distance)
        time.sleep(1)

except KeyboardInterrupt:
    # Clean up the LCD display on exit
    lcd.clear()
    print("Program stopped.")
