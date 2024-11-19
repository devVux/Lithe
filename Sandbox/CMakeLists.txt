cmake_minimum_required(VERSION 3.8)

project(Sandbox)

set(SOURCES
    "Sandbox.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Lithe)

if(LITHE_RENDERER STREQUAL "Metal")
    target_link_libraries(${PROJECT_NAME} PRIVATE LLGL_Metal)
elseif(LITHE_RENDERER STREQUAL "OpenGL")
    target_link_libraries(${PROJECT_NAME} PRIVATE LLGL_OpenGL)
endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${CMAKE_BINARY_DIR}/compile_commands.json" 
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
)
