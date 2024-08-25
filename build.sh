#!/bin/bash

# Parse named parameters
while getopts "c:b:" opt; do
  case $opt in
    c) CONFIGURE_PRESET=$OPTARG ;;
    b) BUILD_PRESET=$OPTARG ;;
    \?) echo "Invalid option -$OPTARG" >&2; exit 1 ;;
  esac
done

echo "Running CMake configuration with preset $CONFIGURE_PRESET..."
cmake --preset $CONFIGURE_PRESET

echo "Building project with preset $BUILD_PRESET..."
cmake --build --preset $BUILD_PRESET --verbose



