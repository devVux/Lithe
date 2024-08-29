cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME "Lithe")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)
find_package(box2d REQUIRED)
find_package(LLGL REQUIRED)

enable_testing()
add_subdirectory("tests")

include_directories(
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/Lithe"
    "${PROJECT_SOURCE_DIR}/include/Lithe/Core"
)

set(SOURCES
	"include/Lithe/Core/EntryPoint.cpp"
)

set(IMGUI_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")
set(IMGUI_SOURCES
    "${IMGUI_SOURCE_DIR}/imgui.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_demo.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_draw.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_tables.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    spdlog::spdlog
    glfw
    glm::glm
    box2d::box2d
	LLGL::LLGL 
)

set(RENDERER_DLLS_NAMES
    "LLGL_OpenGL"
    "LLGL_Direct3D11"
)

foreach(RENDERER ${RENDERER_DLLS_NAMES})
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/${RENDERER}.dll"
			"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RENDERER}.dll"
		)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin/${RENDERER}D.dll"
			"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RENDERER}D.dll"
		)
	endif()
endforeach()