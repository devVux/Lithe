#!/bin/bash

# Exit on any error
set -e

# Path to your vcpkg directory (absolute path)
VCPKG_DIR="$(cd ../vendor/vcpkg && pwd)"


# Check if vcpkg is already bootstrapped
if [ ! -f "$VCPKG_DIR/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    $VCPKG_DIR/bootstrap-vcpkg.sh
fi

echo "Installing dependencies..."
$VCPKG_DIR/vcpkg install

