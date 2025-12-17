#!/bin/bash
# Installation script for audio-multieffect build prerequisites on Debian/Ubuntu
# Run with: bash install-prerequisites.sh

set -e  # Exit on error

echo "=========================================="
echo "Installing audio-multieffect prerequisites"
echo "=========================================="
echo ""

# Check if running on Debian/Ubuntu
if ! command -v apt-get &> /dev/null; then
    echo "Error: This script is for Debian/Ubuntu systems only."
    echo "Please install packages manually for your distribution."
    exit 1
fi

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
    echo "Warning: Running as root. This is not recommended."
    echo "Press Ctrl+C to cancel, or Enter to continue..."
    read
fi

echo "Updating package list..."
sudo apt-get update

echo ""
echo "Installing ARM toolchain and build tools..."
sudo apt-get install -y \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    build-essential \
    git \
    make

echo ""
echo "Installing ST-LINK tools for flashing..."
sudo apt-get install -y stlink-tools

echo ""
echo "Installing debugging tools (optional)..."
sudo apt-get install -y \
    gdb-multiarch \
    openocd

echo ""
echo "=========================================="
echo "Installation complete!"
echo "=========================================="
echo ""

# Verify installation
echo "Verifying ARM toolchain installation..."
if command -v arm-none-eabi-gcc &> /dev/null; then
    echo "✓ ARM GCC found:"
    arm-none-eabi-gcc --version | head -n 1
else
    echo "✗ ARM GCC not found in PATH"
    exit 1
fi

echo ""
if command -v st-flash &> /dev/null; then
    echo "✓ st-flash found:"
    st-flash --version 2>&1 | head -n 1
else
    echo "✗ st-flash not found (optional)"
fi

echo ""
echo "=========================================="
echo "Next steps:"
echo "=========================================="
echo "1. Initialize git submodules (if not done):"
echo "   git submodule update --init --recursive"
echo ""
echo "2. Build the project:"
echo "   make -j\$(nproc)"
echo ""
echo "3. Flash to board (with board connected):"
echo "   make flash"
echo ""
echo "For more information, see BUILD_DEBIAN.md"
echo ""
