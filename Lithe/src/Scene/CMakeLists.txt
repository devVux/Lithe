
add_library(Scene STATIC)

target_compile_features(Scene PUBLIC cxx_std_23)

set_target_properties(Scene PROPERTIES
    DEBUG_POSTFIX "D"
)

## Variables
set(LT_SCENE_INCLUDE_DIR "${LITHE_INCLUDE_DIR}/Lithe/Scene")

## Dependencies
find_package(glm REQUIRED)

target_link_libraries(Scene
    PRIVATE
        glm::glm-header-only
		Core
)


## Includes
target_include_directories(Scene
    PUBLIC
        $<BUILD_INTERFACE:${LITHE_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)


## Sources
target_sources(Scene
	PRIVATE
		"Scene.cpp"
		"PerspectiveCamera.cpp"


    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${LT_SCENE_INCLUDE_DIR}
        FILES
            "${LT_SCENE_INCLUDE_DIR}/Scene.hpp"
            "${LT_SCENE_INCLUDE_DIR}/ResourceCache.hpp"
            "${LT_SCENE_INCLUDE_DIR}/PerspectiveCamera.hpp"
)