cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME "Lithe")
project(${PROJECT_NAME})

find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)
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
    ${Stb_INCLUDE_DIR}
    box2d::box2d
	LLGL::LLGL 
)
