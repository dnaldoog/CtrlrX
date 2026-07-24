#!/usr/bin/env bash
set -e

BUILD_DIR="build"
LOG_FILE="build_log.txt"

echo "========================================"
echo "          CtrlrX Build Script           "
echo "========================================"
echo "1) Quick Build (Keep existing build files)"
echo "2) Clean Build (Wipe build directory)"
echo "========================================"
read -p "Select build mode [1 or 2]: " build_choice

echo ""
echo "----------------------------------------"
echo "1) Single Processor (-j1)   [Best for debugging/logs]"
echo "2) All Processors   (-j$(nproc)) [Fastest build]"
echo "----------------------------------------"
read -p "Select processor mode [1 or 2]: " cpu_choice

# Set thread count based on choice
if [ "$cpu_choice" = "2" ]; then
    JOBS=$(nproc)
else
    JOBS=1
fi

# Handle clean option
if [ "$build_choice" = "2" ]; then
    echo ""
    echo "Clearing $BUILD_DIR..."
    rm -rf "$BUILD_DIR"
fi

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Step 1: Configure with CMake (Debug mode)
echo ""
echo "Configuring CMake (Debug)..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug 2>&1 | tee "$LOG_FILE"

# Step 2: Build target
echo ""
echo "Building target using $JOBS thread(s) (logging to $LOG_FILE)..."
cmake --build "$BUILD_DIR" -j "$JOBS" 2>&1 | tee -a "$LOG_FILE"

echo ""
echo "Build complete! Output saved to $LOG_FILE"
