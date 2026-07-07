import time
from sense_hat import SenseHat

# Initialize Sense HAT
sense = SenseHat()

# Define bin full status messages
rpi4_messages = [
    "Landfill and Plastic/Metal are full, needs to be emptied",
    "Landfill is full, needs to be emptied",
    "Plastic/Metal is full, needs to be emptied"
]

# Function to display messages on Sense HAT
def display_messages():
    for msg in rpi4_messages:
        sense.show_message(msg, scroll_speed=0.1, text_colour=[255, 0, 0])  # >
        time.sleep(1)  # Pause between messages


# Run the script
if __name__ == "__main__":
    display_messages()
