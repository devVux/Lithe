
set(PROJECT_NAME "LitheTests")
cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker ettings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TESTS_SOURCES
	"EventDispatcherTests.cpp"
)

add_executable(Testing ${TESTS_SOURCES})
target_link_libraries(Testing PRIVATE GTest::gtest_main GTest::gmock Lithe)

include(GoogleTest)
gtest_discover_tests(Testing)

