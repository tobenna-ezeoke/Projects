from rpi_lcd import LCD

"""
lcd_simulation.py
-----------------
This script simulates an item classification system using an LCD on a Raspberry Pi.
The user is prompted to place one item at a time via an LCD message.
"""

# Initialize LCD
lcd = LCD()

try:
    # Display message once
    lcd.text(" Place one item at a time", 1)

    # Keep program alive until stopped manually
    while True:
        pass  # Do nothing

except KeyboardInterrupt:
    lcd.clear()
    print("Program stopped.")

except Exception as e:
    lcd.clear()
    print(f"An error occurred: {e}")


