# RPi3 – Web Dashboard Host & Bin Monitoring

This directory contains the logic and interface hosted by **Raspberry Pi 3** for the Smart Bin system.

## Purpose

RPi3 acts as the **user-facing controller** in the system. It does two key things:

1. **Hosts the Web Dashboard** (inside the `WEB/` folder) — used for viewing classified waste, monitoring bin status, and reviewing analytics.
2. **Monitors Paper and Compost bins** using IR sensors (handled in `bin_full_monitor.py`) and logs full-bin events to Firebase.

## Folder Contents

- `WEB/`  
  Contains the full frontend for the Smart Bin Dashboard, built with HTML/CSS/JS + Firebase + Chart.js.  
  → For full page details and screenshots, [see the WEB/README.md](WEB/README.md)

- `bin_full_monitor.py`  
  Monitors IR sensors on RPi3 for **Paper** and **Compost** bins. Sends email alerts and updates Firebase when bins are full.

## How RPi3 Fits Into the Full System

| Device | Responsibilities |
|--------|------------------|
| **RPi1** | Detects objects, classifies waste, sends commands |
| **RPi2** | Executes servo/stepper actions to sort waste |
| **RPi3** | Displays classification results, monitors 2 bins, serves the dashboard |
| **RPi4** | Monitors the other 2 bins (Plastic and Landfill) |

## Technologies Used

- **HTML/CSS/JS** – Web frontend (in `/WEB`)
- **Firebase Realtime DB** – Data logging and retrieval
- **Mailgun API** – Email alerts for full bins
- **Sense HAT** – LED display for bin full signals
- **RPi.GPIO** – IR sensor interaction

## Authors

- **Adeyehun Folahanmi** – 101237546  
- **Dearell Tobenna Ezeoke** – 101245819  

Carleton University  
**SYSC 3010A – Computer Systems Development Project**  
Winter 2025


