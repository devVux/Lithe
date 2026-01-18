cmake_minimum_required(VERSION 3.23)
project(Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
	DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_MakeAvailable(googletest)

add_executable(Tests test.cpp)
target_link_libraries(Tests GTest::gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(Tests)
