# RPi4 – Plastic and Landfill Bin Monitoring

This directory contains the bin monitoring logic for **Raspberry Pi 4** in the Smart Bin system.

## Purpose

RPi4 is responsible for monitoring the **Plastic** and **Landfill** bins using IR sensors. It detects when a bin becomes full, notifies the system, and alerts the manager for action.

## Responsibilities

- Monitors IR sensors connected to the Plastic and Landfill bins.
- Sends real-time data to **Firebase Realtime Database**.
- Uses the **Sense HAT** to visually indicate full bins.
- Sends **email alerts** via the Mailgun API when a bin reaches its threshold.
- Waits for reset commands from Firebase before resuming monitoring.

## Script Included

- `bin_full_monitor.py`  
  - Tracks fill level using GPIO IR sensors  
  - Displays LED matrix warnings  
  - Logs events to Firebase  
  - Sends email notifications  
  - Handles Firebase-based resets

## Dependencies

- `RPi.GPIO`
- `firebase_admin`
- `sense_hat`
- `requests`
- Firebase service account key
- Mailgun API key (pre-configured in the script)

## Part of Smart Bin System

| Raspberry Pi | Role                                      |
|--------------|-------------------------------------------|
| RPi1         | Object detection, classification, command |
| RPi2         | Motor control and waste sorting           |
| RPi3         | Web dashboard + monitoring of 2 bins      |
| **RPi4**     | **Monitoring Plastic and Landfill bins**  |


## Author

**Emeka Anonyei** – 101209704  
Carleton University  
**SYSC 3010A – Computer Systems Development Project**  
Winter 2025
