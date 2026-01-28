cmake_minimum_required(VERSION 3.23)

project(Lithe VERSION 0.0.1)

## Variables
set(LITHE_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LITHE_SOURCE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LITHE_ASSETS_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/assets")

message(STATUS "Assets dir: ${LITHE_ASSETS_DIR}")


## Options
option(LITHE_BUILD_TESTS "Build Lithe tests" OFF)

if(UNIX)
    option(USE_WAYLAND "Use Wayland on Linux" OFF)

    message(STATUS "Using wayland: ${USE_WAYLAND}")
endif()


## Targets
add_library(Lithe INTERFACE)

add_subdirectory(src/Core)
add_subdirectory(src/Renderer)

if(LITHE_BUILD_TESTS)
    message(STATUS "Testing enabled")
    add_subdirectory(src/Tests)
endif()

target_link_libraries(Lithe
    INTERFACE
        Core
        Renderer
)


# Platform / Config Defines
add_library(LithePlatform INTERFACE)

target_compile_definitions(LithePlatform INTERFACE
    $<$<PLATFORM_ID:Windows>:LT_WINDOWS>
    $<$<PLATFORM_ID:Darwin>:LT_MACOS>
    $<$<PLATFORM_ID:Linux>:LT_LINUX>
)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER ${cfg} UPPER_CFG)
        target_compile_definitions(Renderer PRIVATE
            $<$<CONFIG:${cfg}>:LT_${UPPER_CFG}>
        )
    endforeach()
else()
    target_compile_definitions(Renderer PRIVATE
        $<$<CONFIG:Debug>:LT_DEBUG>
        $<$<CONFIG:Release>:LT_RELEASE>
    )
endif()


target_compile_definitions(Core
    PUBLIC
        LithePlatform
)

target_compile_definitions(Renderer
    PRIVATE
        LithePlatform

        $<$<PLATFORM_ID:Windows>:LT_WIN32>
        $<$<PLATFORM_ID:Darwin>:LT_COCOA>
        $<$<AND:$<PLATFORM_ID:Linux>,$<NOT:$<BOOL:${USE_WAYLAND}>>>:LT_X11>
        $<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${USE_WAYLAND}>>:LT_WAYLAND>
)


## Installation
include(GNUInstallDirs)

install(
    TARGETS Lithe Core Renderer
    EXPORT LitheTargets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT LitheTargets
    FILE LitheTargets.cmake
    NAMESPACE Lithe::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lithe
)


## Package Config
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/LitheConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LitheConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lithe
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LitheConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/LitheConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/LitheConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lithe
)
