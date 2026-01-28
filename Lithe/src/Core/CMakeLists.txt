
add_library(Core STATIC)

target_compile_features(Core PUBLIC cxx_std_23)

## Variables
set(LT_CORE_INCLUDE_DIR "${LITHE_INCLUDE_DIR}/Lithe/Core")


## Dependencies
find_package(spdlog REQUIRED)

target_link_libraries(Core
    PUBLIC
        spdlog::spdlog
)


## Includes
target_include_directories(Core
    PUBLIC
        $<BUILD_INTERFACE:${LITHE_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)

## Definitions
target_compile_definitions(Core
    PRIVATE
        LOG_DIR="$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
)

## Sources
target_sources(Core
    PRIVATE
        "Log.cpp"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${LT_CORE_INCLUDE_DIR}
        FILES
            ${LT_CORE_INCLUDE_DIR}/Log.hpp
)