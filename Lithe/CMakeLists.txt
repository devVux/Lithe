cmake_minimum_required(VERSION 3.8)

project(Lithe)

set(SOURCES
    "include/Lithe/Core/EntryPoint.cpp"
    "include/Lithe/Core/Application.cpp"
    "include/Lithe/Core/Renderer.cpp"

    "include/Lithe/Scene/Scene.cpp"
    
  "include/Lithe/Core/Window.mm")

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

target_compile_definitions(${PROJECT_NAME} PUBLIC 
	MODULE_NAME=\"${LITHE_RENDERER}\"
	ASSETS_DIR=\"${CMAKE_SOURCE_DIR}/assets\"
	SHADERS_DIR=\"${PROJECT_SOURCE_DIR}/assets/shaders/\"
)

if(APPLE)
	find_library(COCOA Cocoa)
	find_library(QUARTZCORE QuartzCore)
	find_library(METAL Metal)

	target_link_libraries(${PROJECT_NAME} PRIVATE
		${COCOA}
		${QUARTZCORE}
		${METAL}
	)

	target_compile_definitions(${PROJECT_NAME} PUBLIC
		VERTEX_ENTRY_POINT="vertex_shader"
		VERTEX_PROFILE="1.1"
		FRAGMENT_ENTRY_POINT="fragment_shader"
		FRAGMENT_PROFILE="1.1"

		GLFW_EXPOSE_NATIVE_COCOA
	)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ObjC++")

elseif(WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		VERTEX_ENTRY_POINT=\"\"
		VERTEX_PROFILE=\"\"
		FRAGMENT_ENTRY_POINT=\"\"
		FRAGMENT_PROFILE=\"\"

		GLFW_EXPOSE_NATIVE_WIN32
	)
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		VERTEX_ENTRY_POINT=\"\"
		VERTEX_PROFILE=\"\"
		FRAGMENT_ENTRY_POINT=\"\"
		FRAGMENT_PROFILE=\"\"

		GLFW_EXPOSE_NATIVE_X11
	)
endif()


target_link_libraries(${PROJECT_NAME} PUBLIC
    spdlog::spdlog
    glfw
    glm::glm
    box2d::box2d
    LLGL
)


enable_testing()
add_subdirectory("tests")
