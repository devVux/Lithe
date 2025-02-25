cmake_minimum_required(VERSION 3.21)
project(Lithe VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Standard configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

# Output directories (per-config for multi-config generators)
foreach(OUTPUT_DIR IN ITEMS 
    ARCHIVE_OUTPUT_DIRECTORY
    LIBRARY_OUTPUT_DIRECTORY
    RUNTIME_OUTPUT_DIRECTORY
)
    set(CMAKE_${OUTPUT_DIR}_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
    set(CMAKE_${OUTPUT_DIR}_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")
endforeach()

# Platform-specific configuration
include(CheckIncludeFileCXX)
include(GenerateExportHeader)
include(GNUInstallDirs)

add_subdirectory(Lithe)
add_subdirectory(Sandbox)
add_subdirectory(Editor)

enable_testing()
include(CTest)
