cmake_minimum_required(VERSION 3.21)
project(Lithe VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(CheckIncludeFileCXX)
include(GenerateExportHeader)
include(GNUInstallDirs)

add_subdirectory(Lithe)
add_subdirectory(Sandbox)
#add_subdirectory(Editor)

#enable_testing()
#include(CTest)
add_custom_target(copy-compile-commands
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to source directory"
)
