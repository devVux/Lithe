cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME "Lithe")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(APPLE)
    set(LITHE_RENDERER "Metal" CACHE STRING "Renderer backend")
else()
    set(LITHE_RENDERER "OpenGL" CACHE STRING "Renderer backend")
endif()

include_directories("${CMAKE_SOURCE_DIR}/Lithe/include")

add_subdirectory(Lithe)
add_subdirectory(Sandbox)
add_subdirectory(Editor)