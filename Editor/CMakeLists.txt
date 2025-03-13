cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME "Editor")
project(${PROJECT_NAME})

set(LITHE_BUILD_SHARED ON CACHE BOOL "Build Lithe as shared library" FORCE)

set(SOURCES
	"main.cpp"
)

include_directories(
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
)

set(IMGUI_SOURCES
	"${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp"
)

#add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})
#target_link_libraries(${PROJECT_NAME} PRIVATE Lithe)
