cmake_minimum_required(VERSION 3.21)

project(Sandbox LANGUAGES CXX)

add_executable(Sandbox
    main.cpp
    MyWindow.cpp
)

target_compile_features(Sandbox PRIVATE cxx_std_23)

## Dependencies
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(Sandbox
    PRIVATE
        Lithe
        glfw
        spdlog::spdlog
)

option(USE_WAYLAND "Use Wayland on Linux" OFF)

if(WIN32)
    target_compile_definitions(Sandbox
        PRIVATE
            GLFW_EXPOSE_NATIVE_WIN32
    )
elseif(UNIX AND NOT APPLE)
    if (USE_WAYLAND)
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(WAYLAND REQUIRED wayland-client)

        target_compile_definitions(Sandbox
            PRIVATE 
                GLFW_EXPOSE_NATIVE_WAYLAND
        )

        target_link_libraries(Sandbox PRIVATE wayland-client)

    else()
		find_package(X11 REQUIRED)

        target_compile_definitions(Sandbox
            PRIVATE 
                GLFW_EXPOSE_NATIVE_X11
        )

        target_link_libraries(Sandbox PRIVATE X11)
    endif()

endif()

## Installation
install(TARGETS Sandbox
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES $<TARGET_RUNTIME_DLLS:Sandbox> 
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
