cmake_minimum_required(VERSION 3.23)

project("Lithe" VERSION 0.0.1)

set(LITHE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LITHE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LITHE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

message("Assets dir ${LITHE_ASSETS_DIR}")

add_library(Lithe INTERFACE)

add_subdirectory("./src/Core")
add_subdirectory("./src/Renderer")

if (${LITHE_BUILD_TESTS})
	message("Testing enabled")
	add_subdirectory("./src/Tests/")
endif()

target_link_libraries(Lithe 
	INTERFACE 
		Core
		Renderer
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(USE_X11 "Use X11 on Linux" OFF)
    option(USE_WAYLAND "Use Wayland on Linux" ON)
endif()

target_compile_definitions(Core
	PUBLIC
		$<$<PLATFORM_ID:Windows>:LT_WIN32>
		$<$<PLATFORM_ID:Darwin>:LT_COCOA>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${USE_X11}>>:LT_X11>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${USE_WAYLAND}>>:LT_WAYLAND>
)


target_compile_definitions(Renderer
	PRIVATE
		$<$<CONFIG:Debug>:LT_DEBUG_MODE>
		$<$<CONFIG:Release>:LT_RELEASE_MODE>

		$<$<PLATFORM_ID:Windows>:LT_WINDOWS>
		$<$<PLATFORM_ID:Darwin>:LT_MACOS>
		$<$<PLATFORM_ID:Linux>:LT_LINUX>
	
		$<$<PLATFORM_ID:Windows>:LT_WIN32>
		$<$<PLATFORM_ID:Darwin>:LT_COCOA>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${USE_X11}>>:LT_X11>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${USE_WAYLAND}>>:LT_WAYLAND>
)


install(
	TARGETS 
		Lithe
		Core
		Renderer
	EXPORT LitheTargets
	FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INCLUDE_INCDIR}
)

install(EXPORT LitheTargets
	FILE LitheTargets.cmake
	NAMESPACE Lithe::
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${LITHE_INCLUDE_DIR}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)


include(CMakePackageConfigHelpers) 
configure_package_config_file(
	"cmake/LitheConfig.cmake.in"
	"LitheConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Lithe"
)

write_basic_package_version_file( 
	"LitheConfigVersion.cmake" 
	VERSION ${PROJECT_VERSION} 
	COMPATIBILITY AnyNewerVersion 
)
