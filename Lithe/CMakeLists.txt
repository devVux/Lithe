cmake_minimum_required(VERSION 3.8)

project(Lithe)

set(SOURCES
    "include/Lithe/Core/EntryPoint.cpp"
    "include/Lithe/Core/Application.cpp"
    "include/Lithe/Core/Renderer.cpp"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/Lithe"
    "${CMAKE_SOURCE_DIR}/vendor/llgl/include"
)

find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)
find_package(box2d REQUIRED)

if(LITHE_RENDERER STREQUAL "Metal")
    set(LLGL_BUILD_RENDERER_METAL 1 CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_VULKAN 0 CACHE BOOL "" FORCE)
elseif(LITHE_RENDERER STREQUAL "OpenGL")
    set(LLGL_BUILD_RENDERER_OPENGL 1 CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D11 0 CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D12 0 CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_VULKAN 0 CACHE BOOL "" FORCE)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/llgl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC MODULE_NAME=\"${LITHE_RENDERER}\")
target_link_libraries(${PROJECT_NAME} PUBLIC
    spdlog::spdlog
    glfw
    glm::glm
    box2d::box2d
    LLGL
)

enable_testing()
add_subdirectory("tests")
