#!/bin/bash

# Define the virtual environment directory name
VENV_DIR="venv"

# Create a virtual environment if it doesn't exist
if [ ! -d "$VENV_DIR" ]; then
    echo "Creating virtual environment..."
    python3 -m venv "$VENV_DIR"
fi

# Check if running inside a script
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "Please run this script using: source setup_env.sh"
    exit 1
fi

# Activate the virtual environment
echo "Activating virtual environment..."
source "$VENV_DIR/bin/activate"

# Upgrade pip and install rpi_lcd
echo "Installing rpi_lcd..."
pip install --upgrade pip
pip install rpi_lcd

echo "Setup complete. Virtual environment is ready."

