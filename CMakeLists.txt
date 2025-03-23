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
    set(CMAKE_${OUTPUT_DIR}_DEBUG "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_${OUTPUT_DIR}_RELEASE "${CMAKE_BINARY_DIR}/bin")
endforeach()

# Platform-specific configuration
include(CheckIncludeFileCXX)
include(GenerateExportHeader)
include(GNUInstallDirs)

add_subdirectory(Lithe)
add_subdirectory(Sandbox)
add_subdirectory(Editor)

include(CTest)

# Copy compile_commands.json to the source directory
if (NOT MSVC)
	add_custom_target(copy-compile-commands ALL
	  COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_BINARY_DIR}/compile_commands.json
		${CMAKE_SOURCE_DIR}/compile_commands.json
	  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
	  COMMENT "Copying compile_commands.json to source directory"
	)
endif()
