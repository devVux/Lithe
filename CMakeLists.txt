
cmake_minimum_required (VERSION 3.8)
	
set(PROJECT_NAME "Lithe")
project(${PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories("${CMAKE_SOURCE_DIR}/Lithe/include")

add_subdirectory(Lithe)
add_subdirectory(Sandbox)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
