project(Sandbox LANGUAGES C CXX)

add_executable(Sandbox
    "Sandbox.cpp"
)

target_link_libraries(Sandbox PRIVATE
    Lithe
)

set_target_properties(Sandbox PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
