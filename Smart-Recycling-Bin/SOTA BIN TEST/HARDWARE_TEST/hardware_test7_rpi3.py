import time
from sense_hat import SenseHat

# Initialize Sense HAT
sense = SenseHat()

# Define bin full status messages
rpi3_messages = [
    "Compost and Paper are full, needs to be emptied",
    "Compost is full, needs to be emptied",
    "Paper is full, needs to be emptied",
]


# Function to display messages on Sense HAT
def display_messages():
    for msg in rpi3_messages:
        sense.show_message(msg, scroll_speed=0.1, text_colour=[255, 0, 0])  # Red text
        time.sleep(1)  # Pause between messages

# Run the script
if __name__ == "__main__":
    display_messages()
