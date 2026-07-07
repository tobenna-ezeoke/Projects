import time
import random
from rpi_lcd import LCD

# Initialize LCD
lcd = LCD()


def simulate():
    lcd.text(" Place one item at a time, ready for new item", 1)
    time.sleep(3)
    lcd.clear()
    lcd.text("Wait, sorting ...", 1)
    time.sleep(3)
    lcd.clear()
   

try:
    while True:
        # Display on LCD screen
        simulate()
        time.sleep(1)

except KeyboardInterrupt:
    # Clean up the LCD display on exit
    lcd.clear()
    print("Program stopped.")
