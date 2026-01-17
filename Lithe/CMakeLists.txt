cmake_minimum_required(VERSION 3.23)

project("Lithe" VERSION 0.0.1)

set(LITHE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(Lithe INTERFACE)

add_subdirectory("./src/Core/")
#add_subdirectory("./src/Core/")

target_link_libraries(Lithe 
	INTERFACE 
		Core
)


install(
	TARGETS 
		Lithe
		Core
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
