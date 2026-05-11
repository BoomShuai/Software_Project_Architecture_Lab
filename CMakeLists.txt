cmake_minimum_required(VERSION 3.14)
project(GildedRoseBackend)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# =====================================================
# Main application sources (exclude main.cpp for tests)
# =====================================================
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX "src/main\\.cpp$")

# Main executable
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
add_executable(GildedRoseBackend ${ALL_SOURCES})
target_link_libraries(GildedRoseBackend sqlite3)

# =====================================================
# Testing configuration with GoogleTest
# =====================================================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()

    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Coverage flags (gcov)
    if(ENABLE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()

    # Collect all test source files
    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

    add_executable(GildedRoseTests ${TEST_SOURCES} ${LIB_SOURCES})
    target_include_directories(GildedRoseTests PRIVATE src)
    target_link_libraries(GildedRoseTests GTest::gtest_main sqlite3)

    include(GoogleTest)
    gtest_discover_tests(GildedRoseTests)
endif()
