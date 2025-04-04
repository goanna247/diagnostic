
#!/bin/bash

# This script installs the required dependencies and sets up the project on Ubuntu.

# Exit if any command fails
set -e

# Function to check if a package is installed
check_package_installed() {
    dpkg -l | grep -qw "$1" || return 1
    return 0
}

# Function to install packages
install_packages() {
    sudo apt-get update
    for PACKAGE in "${REQUIRED_PACKAGES[@]}"; do
        if check_package_installed "$PACKAGE"; then
            echo "$PACKAGE is already installed."
        else
            echo "Installing $PACKAGE..."
            sudo apt-get install -y "$PACKAGE"
        fi
    done
}

# List of required packages
REQUIRED_PACKAGES=(
    "build-essential"
    "cmake"
    "pkg-config"
    "libglib2.0-dev"
    "libdbus-1-dev"
    "libudev-dev"
    "libbluetooth-dev"
    "libsbc-dev"
    "libspeexdsp-dev"
    "libssl-dev"
    "libncurses5-dev"
    "libreadline-dev"
    "libcurl4-openssl-dev"
    "libz-dev"
)

# Install required packages
echo "Installing required packages..."
install_packages

# Set up BlueZ
if [ ! -d "~/Downloads/new_folder/bluez-5.80" ]; then
    echo "BlueZ source directory not found. Cloning BlueZ repository..."
    git clone https://github.com/bluez/bluez.git ~/Downloads/new_folder/bluez-5.80
    cd ~/Downloads/new_folder/bluez-5.80
    ./bootstrap-configure
    make -j$(nproc)
    sudo make install
    echo "BlueZ installed successfully!"
else
    echo "BlueZ is already present."
fi

# Reminder for post-installation steps
echo "Setup complete! All necessary libraries and dependencies are installed."
echo "For post-installation, you may need to set environment variables like PKG_CONFIG_PATH."

# If there are additional dependencies (e.g., Python, Node.js), add them below:
# Example: python3 -m pip install -r requirements.txt
# Example for Node.js: npm install

echo "Installation complete!"
