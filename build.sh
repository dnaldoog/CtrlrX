#!/bin/bash

# Prompt the user for build type
echo "Select build type:"
echo "1) Release"
echo "2) Debug"
read -p "Enter choice [1 or 2]: " choice

case $choice in
    1)
        BUILD_TYPE="Release"
        ;;
    2)
        BUILD_TYPE="Debug"
        ;;
    *)
        echo "Invalid selection. Exiting."
        exit 1
        ;;
esac

echo "Starting $BUILD_TYPE build..."

# Run the build commands
rm -fr build && \
mkdir build && \
cd build && \
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE .. && \
make -j$(nproc)
