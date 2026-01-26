cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LT_RENDERER_INCLUDE_DIR "${LITHE_INCLUDE_DIR}/Lithe/Renderer")

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
message("Compiling shaders to ${SHADER_OUTPUT_DIR}")

# TODO: might wanna decide build type
add_library(Renderer STATIC)

find_package(Vulkan REQUIRED)
find_program(GLSLC glslc REQUIRED)


file(GLOB_RECURSE SHADER_SOURCES 
	"${LITHE_ASSETS_DIR}/shaders/*.glsl"
)

foreach(SHADER ${SHADER_SOURCES})
	get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
	
	# Determine shader stage from filename
	if(SHADER_NAME MATCHES "vert")
		set(SHADER_STAGE "vertex")
	elseif(SHADER_NAME MATCHES "frag")
		set(SHADER_STAGE "fragment")
	elseif(SHADER_NAME MATCHES "comp")
		set(SHADER_STAGE "compute")
	elseif(SHADER_NAME MATCHES "geom")
		set(SHADER_STAGE "geometry")
	elseif(SHADER_NAME MATCHES "tesc")
		set(SHADER_STAGE "tesscontrol")
	elseif(SHADER_NAME MATCHES "tese")
		set(SHADER_STAGE "tesseval")
	else()
		message(WARNING "Unknown shader stage for: ${SHADER_NAME}")
		continue()
	endif()
	
	set(SPIRV "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
	
	add_custom_command(
		COMMENT "Compiling ${SHADER_STAGE} shader: ${SHADER_NAME}"
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
		COMMAND ${GLSLC} -fshader-stage=${SHADER_STAGE} ${SHADER} -o ${SPIRV}
		DEPENDS ${SHADER}
	)
	list(APPEND SPIRV_BINARIES ${SPIRV})
endforeach()
add_custom_target(Shaders ALL DEPENDS ${SPIRV_BINARIES})


target_compile_definitions(Renderer 
	PRIVATE
		SHADERS_DIR="${SHADER_OUTPUT_DIR}"
)

target_sources(Renderer
	PRIVATE
		"RenderSystem.cpp"

	PUBLIC
	  FILE_SET HEADERS
	  BASE_DIRS ${LT_RENDERER_INCLUDE_DIR}
	  FILES
		"${LT_RENDERER_INCLUDE_DIR}/ForwardDecls.hpp"
		"${LT_RENDERER_INCLUDE_DIR}/RenderSystem.hpp"
)

add_dependencies(Renderer Shaders)

target_link_libraries(Renderer
	PRIVATE
		Vulkan::Vulkan
		Core
)

