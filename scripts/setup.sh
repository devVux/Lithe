#!/bin/bash

# Path to your vcpkg directory
VCPKG_DIR="../vendor"

# Check if vcpkg is already bootstrapped
if [ ! -f "$VCPKG_DIR/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    $VCPKG_DIR/bootstrap-vcpkg.sh
fi

# Install dependencies if needed
$VCPKG_DIR/vcpkg install

# Run CMake with the vcpkg toolchain
cmake ..
