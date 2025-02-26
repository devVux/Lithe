cmake_minimum_required(VERSION 3.21)
project(Lithe LANGUAGES C CXX)

# ----------------------------
# Configuration Options
# ----------------------------
option(LITHE_BUILD_SHARED "Build as shared library" OFF)
option(LITHE_BUILD_TESTS "Build tests" ON)


# ----------------------------
# Source Management
# ----------------------------
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR 
	"Lithe",
	"Lithe/Core",
	"Lithe/Events",
	"Lithe/Scene",
	"Lithe/Utils"
)

# ----------------------------
# Dependency Configuration
# ----------------------------
# LLGL Renderer Selection
set(LITHE_RENDERER "Auto" CACHE STRING "Renderer backend (Auto, OpenGL, Metal, etc.)")
set_property(CACHE LITHE_RENDERER PROPERTY STRINGS "Auto;OpenGL;Metal;Direct3D11;Vulkan")

if(LITHE_RENDERER STREQUAL "Auto")
    if(APPLE)
        set(LITHE_RENDERER "Metal")
    elseif(WIN32)
        set(LITHE_RENDERER "Direct3D11")
    else()
        set(LITHE_RENDERER "OpenGL")
    endif()
endif()

# ----------------------------
# LLGL Configuration
# ----------------------------
set(LLGL_DIR "${CMAKE_SOURCE_DIR}/vendor/llgl")
set(LLGL_BIN_DIR "${CMAKE_BINARY_DIR}/vendor/llgl")

if(NOT EXISTS "${LLGL_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "LLGL submodule not found at ${LLGL_DIR}. Run:\n  git submodule update --init --recursive")
endif()

set(LLGL_BUILD_RENDERER_OPENGL OFF)
set(LLGL_BUILD_RENDERER_METAL OFF)
set(LLGL_BUILD_RENDERER_DIRECT3D11 OFF)
set(LLGL_BUILD_RENDERER_VULKAN OFF)

if(LITHE_RENDERER STREQUAL "OpenGL")
    set(LLGL_BUILD_RENDERER_OPENGL ON )
elseif(LITHE_RENDERER STREQUAL "Metal")
    set(LLGL_BUILD_RENDERER_METAL ON)
elseif(LITHE_RENDERER STREQUAL "Direct3D11")
    set(LLGL_BUILD_RENDERER_DIRECT3D11 ON)
elseif(LITHE_RENDERER STREQUAL "Vulkan")
    set(LLGL_BUILD_RENDERER_VULKAN ON)
endif()

set(LLGL_BUILD_EXAMPLES OFF CACHE BOOL "")
set(LLGL_BUILD_TESTS OFF CACHE BOOL "")
set(LLGL_BUILD_TOOLS OFF CACHE BOOL "")

add_subdirectory(
    "${LLGL_DIR}"
    "${LLGL_BIN_DIR}"
)

# ----------------------------
# Dependency Management
# ----------------------------
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
#find_package(Stb CONFIG REQUIRED)

# ----------------------------
# Target Configuration
# ----------------------------
# Platform-specific source filtering
set(PLATFORM_SOURCES "")
if(WIN32)
	list(APPEND PLATFORM_SOURCES "src/Platform/Windows/Window.cpp")
elseif(APPLE)
    list(APPEND PLATFORM_SOURCES "src/Platform/MacOS/Window.mm")
	set_source_files_properties(
		${PLATFORM_SOURCES}
		PROPERTIES
		SKIP_PRECOMPILE_HEADERS ON
	)
elseif(UNIX AND NOT APPLE)  # Linux
	list(APPEND PLATFORM_SOURCES "src/Platform/Linux/Window.cpp")
endif()

add_library(Lithe ${LIB_TYPE} 
	"src/pch.cpp"

    "src/Core/Application.cpp"
	"src/Core/EntryPoint.cpp"
    "src/Core/Renderer.cpp"

	${PLATFORM_SOURCES}

    "src/Scene/OrthographicCamera.cpp"
    "src/Scene/PerspectiveCamera.cpp"
    "src/Scene/Scene.cpp"
)

target_compile_definitions(Lithe PRIVATE LITHE_RENDERER="${LITHE_RENDERER}")
target_compile_definitions(Lithe PUBLIC 
    ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
)

# ----------------------------
# Precompiled Header
# ----------------------------
target_precompile_headers(Lithe PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h>
)

# ----------------------------
# Include Directories
# ----------------------------
target_include_directories(Lithe PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
foreach(dir IN LISTS INC_DIR)
    target_include_directories(Lithe PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${dir}>
        $<INSTALL_INTERFACE:include/${dir}>
    )
endforeach()

target_include_directories(Lithe PUBLIC
    $<BUILD_INTERFACE:${LLGL_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ----------------------------
# Platform-Specific Configuration
# ----------------------------
if(APPLE)
    target_link_libraries(Lithe PRIVATE
        "-framework Cocoa"
		"-framework Metal"
        "-framework QuartzCore"
    )
    target_compile_definitions(Lithe PRIVATE
        GLFW_EXPOSE_NATIVE_COCOA
    )
elseif(WIN32)
    target_compile_definitions(Lithe PRIVATE
        GLFW_EXPOSE_NATIVE_WIN32
    )
else()
    target_compile_definitions(Lithe PRIVATE
        GLFW_EXPOSE_NATIVE_X11
    )
endif()

# ----------------------------
# Dependency Linking
# ----------------------------
target_link_libraries(Lithe PRIVATE LLGL)
target_link_libraries(Lithe PRIVATE glfw)
target_link_libraries(Lithe PUBLIC  glm::glm)
target_link_libraries(Lithe PUBLIC  box2d::box2d)
target_link_libraries(Lithe PRIVATE spdlog::spdlog)

# ----------------------------
# Compiler Configuration
# ----------------------------
target_compile_features(Lithe PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(Lithe PRIVATE /W4 /permissive-)
else()
    target_compile_options(Lithe PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ----------------------------
# Testing
# ----------------------------
if(LITHE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ----------------------------
# Installation
# ----------------------------
include(GNUInstallDirs)
install(TARGETS Lithe
    EXPORT LitheTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
