cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME "Sandbox")
project(${PROJECT_NAME})

set(SOURCES
	"Sandbox.cpp"
)

include_directories("${CMAKE_SOURCE_DIR}/Lithe/include")


add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Lithe)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
