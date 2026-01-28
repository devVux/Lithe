
add_library(Renderer STATIC)

target_compile_features(Renderer PUBLIC cxx_std_23)


## Variables
set(LT_RENDERER_INCLUDE_DIR "${LITHE_INCLUDE_DIR}/Lithe/Renderer")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

message(STATUS "Compiling shaders to ${SHADER_OUTPUT_DIR}")


## Dependencies
find_package(Vulkan REQUIRED)
find_program(GLSLC glslc REQUIRED)

target_link_libraries(Renderer
    PRIVATE
        Vulkan::Vulkan
        Core
)


## Includes
target_include_directories(Renderer
    PUBLIC
        $<BUILD_INTERFACE:${LITHE_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)

## Definitions


## Sources
target_sources(Renderer
    PRIVATE
        RenderSystem.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${LT_RENDERER_INCLUDE_DIR}
        FILES
            ${LT_RENDERER_INCLUDE_DIR}/ForwardDecls.hpp
            ${LT_RENDERER_INCLUDE_DIR}/RenderSystem.hpp
)


## Shader Compilation
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS
    "${LITHE_ASSETS_DIR}/shaders/*.glsl"
)

set(SPIRV_BINARIES)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)

    if(SHADER_NAME MATCHES "vert")
        set(STAGE vertex)
    elseif(SHADER_NAME MATCHES "frag")
        set(STAGE fragment)
    elseif(SHADER_NAME MATCHES "comp")
        set(STAGE compute)
    elseif(SHADER_NAME MATCHES "geom")
        set(STAGE geometry)
    elseif(SHADER_NAME MATCHES "tesc")
        set(STAGE tesscontrol)
    elseif(SHADER_NAME MATCHES "tese")
        set(STAGE tesseval)
    else()
        message(WARNING "Unknown shader stage: ${SHADER_NAME}")
        continue()
    endif()

    set(SPIRV "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
        COMMAND ${GLSLC} -fshader-stage=${STAGE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        VERBATIM
    )

    list(APPEND SPIRV_BINARIES ${SPIRV})
endforeach()

add_custom_target(RendererShaders DEPENDS ${SPIRV_BINARIES})
add_dependencies(Renderer RendererShaders)

target_compile_definitions(Renderer
    PRIVATE SHADERS_DIR="${SHADER_OUTPUT_DIR}"
)
