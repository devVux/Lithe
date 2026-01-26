cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 REQUIRED)

if (UNIX)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(WAYLAND REQUIRED wayland-client)

	target_compile_definitions(Sandbox
		PRIVATE
			GLFW_EXPOSE_NATIVE_WAYLAND
	)
endif()

add_executable(Sandbox
    "main.cpp"
	"Window.cpp"
)


target_link_libraries(Sandbox
	PRIVATE
		Lithe
		glfw
)

if (UNIX)
target_link_libraries(Sandbox
	PRIVATE
		wayland-client
)
#endif

install(TARGETS Sandbox
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
