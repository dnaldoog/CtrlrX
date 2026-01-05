#!/bin/bash

# Define the build directory path
BUILD_DIR="$HOME/Documents/CtrlrX/build"

# Prompt the user for build type
echo "Select build type:"
echo "1) Full Release (Clean + CMake)"
echo "2) Full Debug (Clean + CMake)"
echo "3) Incremental Rebuild (Make Clean + Make)"
echo "4) Quick Rebuild (Make only)"
read -p "Enter choice [1-4]: " choice

case $choice in
    1)
        # Full Release
        rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR" || exit
        cmake -DCMAKE_BUILD_TYPE=Release ..
        make -j$(nproc)
        ;;
    2)
        # Full Debug
        rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR" || exit
        cmake -DCMAKE_BUILD_TYPE=Debug ..
        make -j$(nproc)
        ;;
    3)
        # Incremental Clean
        cd "$BUILD_DIR" || { echo "Build dir not found. Run 1 or 2 first."; exit 1; }
        make clean
        make -j$(nproc)
        ;;
    4)
        # Quick Rebuild
        cd "$BUILD_DIR" || { echo "Build dir not found. Run 1 or 2 first."; exit 1; }
        make -j$(nproc)
        ;;
    *)
        echo "Invalid selection. Exiting."
        exit 1
        ;;
esac
