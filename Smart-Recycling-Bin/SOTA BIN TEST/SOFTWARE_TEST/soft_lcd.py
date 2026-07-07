import time
from rpi_lcd import LCD

# Initialize LCD
lcd = LCD()

def simulate():
    """Displays sorting messages on the LCD screen."""
    print("[INFO] Displaying: 'Place one item at a time, ready for new item'")
    lcd.text(" Place one item at a time, ready for new item", 1)
    time.sleep(3)
    lcd.clear()

    print("[INFO] Displaying: 'wait,sorting'")
    lcd.text("wait,sorting", 1)
    time.sleep(3)
    lcd.clear()

    print("[INFO] Simulation completed.")
