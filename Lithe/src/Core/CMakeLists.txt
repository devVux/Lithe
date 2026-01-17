cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LT_CORE_INCLUDE_DIR "${LITHE_INCLUDE_DIR}/Lithe/Core")

add_library(Core STATIC)

find_package(spdlog)

target_compile_definitions(Core 
	PRIVATE
		$<$<CONFIG:Debug>:LOG_DIR="${CMAKE_BINARY_DIR}">
		$<$<CONFIG:Release>:LOG_DIR="${CMAKE_INSTALL_PREFIX}">
)
message("Log dir ${LOG_DIR}")

target_sources(Core
    PRIVATE
      "Log.cpp"

    PUBLIC
      FILE_SET HEADERS
      BASE_DIRS ${LT_CORE_INCLUDE_DIR}
      FILES
        "${LT_CORE_INCLUDE_DIR}/Log.hpp"
)

target_link_libraries(Core
    PUBLIC

    PRIVATE
      spdlog::spdlog
)

