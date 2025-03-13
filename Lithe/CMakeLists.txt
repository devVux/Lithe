cmake_minimum_required(VERSION 3.21)
project(Lithe LANGUAGES C CXX)

# ----------------------------
# Configuration Options
# ----------------------------
option(LITHE_BUILD_SHARED "Build as shared library" OFF)
option(LITHE_BUILD_TESTS "Build tests" OFF)

# ----------------------------
# Source Management
# ----------------------------
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR 
	"Lithe"
	"Lithe/Core"
	"Lithe/Events"
	"Lithe/Scene"
	"Lithe/Utils"
)

# ----------------------------
# Dependency Configuration
# ----------------------------
# LLGL Renderer Selection
set(LITHE_RENDERER_DEFAULT "OpenGL")
if(APPLE)
    set(LITHE_RENDERER_DEFAULT "Metal")
elseif(WIN32)
    set(LITHE_RENDERER_DEFAULT "Direct3D11")
endif()

set(LITHE_RENDERER "${LITHE_RENDERER_DEFAULT}" CACHE STRING "Renderer backend (OpenGL, Metal, etc.)" FORCE)
set_property(CACHE LITHE_RENDERER PROPERTY STRINGS "OpenGL;Metal;Direct3D11;Vulkan")

# ----------------------------
# LLGL Configuration
# ----------------------------
set(LLGL_DIR "${CMAKE_SOURCE_DIR}/vendor/llgl")
set(LLGL_BIN_DIR "${CMAKE_BINARY_DIR}/vendor/llgl")

if(NOT EXISTS "${LLGL_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "LLGL submodule not found at ${LLGL_DIR}. Run:\n  git submodule update --init --recursive")
endif()

set(LLGL_BUILD_RENDERER_OPENGL OFF CACHE BOOL "Build renderer OpenGL" FORCE)
set(LLGL_BUILD_RENDERER_METAL OFF CACHE BOOL "Build renderer Metal" FORCE)
set(LLGL_BUILD_RENDERER_DIRECT3D11 OFF CACHE BOOL "Build renderer Direct3D11" FORCE)
set(LLGL_BUILD_RENDERER_VULKAN OFF CACHE BOOL "Build renderer Vulkan" FORCE)

if(LITHE_RENDERER STREQUAL "OpenGL")
   set(LLGL_BUILD_RENDERER_OPENGL ON CACHE BOOL "Build renderer OpenGL" FORCE)
elseif(LITHE_RENDERER STREQUAL "Metal")
   set(LLGL_BUILD_RENDERER_METAL ON CACHE BOOL "Build renderer Metal" FORCE)
elseif(LITHE_RENDERER STREQUAL "Direct3D11")
   set(LLGL_BUILD_RENDERER_DIRECT3D11 ON CACHE BOOL "Build renderer Direct3D11" FORCE)
elseif(LITHE_RENDERER STREQUAL "Vulkan")
   set(LLGL_BUILD_RENDERER_VULKAN ON CACHE BOOL "Build renderer Vulkan" FORCE)
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
	list(APPEND PLATFORM_SOURCES "src/Platform/Windows/Win32Window.cpp")
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

set(SOURCES 
	"src/pch.cpp"

    "src/Core/Application.cpp"
	"src/Core/EntryPoint.cpp"
    "src/Core/Renderer.cpp"
    
	${PLATFORM_SOURCES}

    "src/Scene/OrthographicCamera.cpp"
    "src/Scene/PerspectiveCamera.cpp"
    "src/Scene/Scene.cpp"
)
if(LITHE_BUILD_SHARED)	
	add_library(Lithe SHARED ${SOURCES})
else()
	add_library(Lithe STATIC ${SOURCES})
endif()

target_compile_definitions(Lithe PRIVATE LITHE_RENDERER="${LITHE_RENDERER}")
target_compile_definitions(Lithe PUBLIC 
    ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
generate_export_header(Lithe
    BASE_NAME LITHE
	EXPORT_MACRO_NAME LITHE_EXPORT
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/export/Export.h"
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
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
	$<INSTALL_INTERFACE:include>
)

foreach(dir IN LISTS INC_DIR)
    target_include_directories(Lithe PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${dir}>
        $<INSTALL_INTERFACE:include/${dir}>
    )
endforeach()

target_include_directories(Lithe PRIVATE
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
else()
endif()

# ----------------------------
# Dependency Linking
# ----------------------------
target_link_libraries(Lithe PRIVATE LLGL)
target_link_libraries(Lithe PRIVATE glfw)
target_link_libraries(Lithe PUBLIC  glm::glm)
target_link_libraries(Lithe PUBLIC  box2d::box2d)
target_link_libraries(Lithe PRIVATE spdlog::spdlog)
target_link_libraries(Lithe PRIVATE fmt::fmt)

# ----------------------------
# Compiler Configuration
# ----------------------------
if(MSVC)
	target_compile_options(Lithe PRIVATE /W4 /permissive-)
else()
    target_compile_options(Lithe PRIVATE -Wno-all)
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
