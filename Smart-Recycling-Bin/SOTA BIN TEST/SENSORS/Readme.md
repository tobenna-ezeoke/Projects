# Ultrasonic_sensor.py

## Overview
This Python script logs ultrasonic sensor readings into an SQLite database and monitors bin status using a Raspberry Pi. It is designed to track waste bin levels and detect when a bin is full.

## Features
- Initializes an SQLite database (`rpi3_sensor_logs.db`) to store distance readings.
- Uses two ultrasonic sensors to measure distances in centimeters.
- Logs distance data into the database.
- Implements a detection mechanism to determine if a bin is full.
- Runs continuously, monitoring bin status in real time.
- Graceful shutdown on keyboard interrupt (`Ctrl+C`).

## Requirements
- Raspberry Pi with Raspbian (64-bit OS recommended)
- Python 3
- SQLite3
- `RPi.GPIO` library

## Installation
1. Clone the repository or copy the script to your Raspberry Pi.
2. Install required dependencies:
   ```sh
   sudo apt update && sudo apt install python3 python3-pip sqlite3
   pip install RPi.GPIO
   ```
3. Ensure your ultrasonic sensors are properly connected to the Raspberry Pi GPIO pins.

## Usage
1. Run the script:
   ```sh
   python3 rpi3_sensor_logger.py
   ```
2. The script will continuously measure distances and log them into the database.
3. If a bin is detected as full for 5 consecutive readings, a warning message is displayed.
4. Stop the script using `Ctrl+C`.

## Database Structure
- **Table: `classifications`**
  - `id` (INTEGER, Primary Key)
  - `timestamp` (DATETIME, Auto-filled)
  - `bin` (INTEGER, Values: 1-4)
  - `distance` (INTEGER, Values: 0-400 cm)

## GPIO Pin Configuration
- **Sensor 1**
  - Trigger: GPIO 23
  - Echo: GPIO 24
- **Sensor 2**
  - Trigger: GPIO 20
  - Echo: GPIO 21




