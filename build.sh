#!/bin/bash

# Parse named parameters
while getopts "c:b:" opt; do
  case $opt in
    c) CONFIGURE_PRESET=$OPTARG ;;
    b) BUILD_PRESET=$OPTARG ;;
    \?) echo "Invalid option -$OPTARG" >&2; exit 1 ;;
  esac
done

# Ensure CONFIGURE_PRESET and BUILD_PRESET are set
if [ -z "$CONFIGURE_PRESET" ]; then
  error_exit "Configuration preset (-c) is required"
fi

if [ -z "$BUILD_PRESET" ]; then
  error_exit "Build preset (-b) is required"
fi

echo "Running CMake configuration with preset $CONFIGURE_PRESET..."
cmake --preset $CONFIGURE_PRESET

echo "Building project with preset $BUILD_PRESET..."
cmake --build --preset $BUILD_PRESET



