# RPi1 - Smart Bin Controller

This directory contains the software components running on **Raspberry Pi 1** for the Smart Waste Sorting Bin system. RPi1 is responsible for object detection, material classification, sensor integration, database logging, and communication with both a cloud service (Firebase) and a secondary device (RPi2).


## Contents

- `lcd_simulation.py`: A simple script that displays a static message on the LCD screen prompting the user to place one item at a time.
- `classifier_and_commander.py`: The main application that:
  - Captures live video feed using PiCamera2.
  - Learns a background reference image and detects changes to identify new objects.
  - Reads values from moisture and metal sensors.
  - Classifies materials using rule-based logic.
  - Logs event data into a local SQLite database.
  - Sends classification results to Firebase Realtime Database.
  - Sends instructions to Raspberry Pi 2 over WebSocket for mechanical sorting.


## Requirements

Ensure the following packages are installed on your Raspberry Pi:

```bash
pip install opencv-python firebase-admin RPi.GPIO adafruit-circuitpython-ads1x15 websockets
```

Additional dependencies:
- `picamera2` (preinstalled on recent Raspberry Pi OS images)
- `sqlite3` (standard Python library)
- `numpy`
- `cv2` (OpenCV)

Hardware dependencies:
- Raspberry Pi with GPIO support
- PiCamera2 module
- 2 Moisture sensors connected to ADC channels
- Metal detection sensor (connected to GPIO 4)
- LCD screen
- Internet connection for Firebase updates


## Setup Instructions

1. Place the Firebase service account credentials in this directory and rename the file:
   ```
   firebase_key.json
   ```
2. **Do not upload this file to GitHub.** Add the following line to `.gitignore`:
   ```
   firebase_key.json
   ```

3. Update `classifier_and_commander.py` with the correct WebSocket URI for your RPi2 (if different).


## How to Run

From the `RPi1/` directory, execute the main script:

```bash
python3 classifier_and_commander.py
```

- The system will capture a background image at startup.
- After the bin is empty and motion is detected, it will:
  - Delay briefly for user to finish interaction
  - Capture the item
  - Classify the material using visual and sensor data
  - Send commands and log the event

### Controls

- Press **`r`** to relearn the background manually.
- Press **`q`** to quit the application safely.


## Database Structure

All data is stored locally in an SQLite database at:

```
/home/uchennaobikwelu/sysc3010-project-l2-g3/SOTA BIN MAIN/RPi1/sota_bin_log.db
```

Tables include:
- `presence_events`: Detected object presence timestamps
- `metal_sensor_events`: Metal detection results
- `moisture_levels`: Raw sensor readings from both moisture sensors
- `classified_items`: Final classification result (material + bin) with timestamps


## WebSocket Communication

This script sends classification results to a sorting controller running on RPi2:

```
ws://172.17.182.70:8765
```

The JSON payload sent includes:
```json
{
  "type": "Plastic",
  "bin": "Plastic/Metal bin"
}
```

This allows RPi2 to sort the item accordingly.


## Firebase Realtime Database

Classification results are also pushed to Firebase under the `/classification` path:

Example entry:
```json
{
  "material": "Paper",
  "bin": "Paper bin",
  "timestamp": "2025-04-06 15:30:12"
}
```

Make sure your Firebase project is set up and the credentials file is correct.


## Future Improvements

- **LCD Display Enhancements**: Instead of a static prompt, update the LCD dynamically to show real-time status such as:
  - "Sorting in progress..."
  - "Ready for next item"
- **Object Removal Detection**: Add a check to confirm item removal before allowing the next classification cycle.
- **Hardware Feedback**: Provide audio or visual cues (e.g., buzzer or LED) after classification.


## Author

Uchenna Obikwelu  
101241887  
Carleton University  
SYSC 3010A – Computer Systems Development Project  
Winter 2025

