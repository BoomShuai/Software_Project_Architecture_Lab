cmake_minimum_required(VERSION 3.14)
project(GildedRoseBackend)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# =====================================================
# Optional Redis cache backend (Experiment 4: scalability)
# Enable with: cmake -DUSE_REDIS=ON (requires hiredis).
# When OFF, CacheService falls back to an in-process LRU cache, so the
# default build and the test build have no external dependency.
# =====================================================
option(USE_REDIS "Build the Redis cache backend via hiredis" OFF)

# =====================================================
# Main application sources (exclude main.cpp for tests)
# =====================================================
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX "src/main\\.cpp$")

# Main executable
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
add_executable(GildedRoseBackend ${ALL_SOURCES})
target_link_libraries(GildedRoseBackend sqlite3)

if(USE_REDIS)
    find_path(HIREDIS_HEADER hiredis/hiredis.h
        PATHS /opt/homebrew/include /usr/local/include)
    find_library(HIREDIS_LIB hiredis
        PATHS /opt/homebrew/lib /usr/local/lib)
    if(HIREDIS_HEADER AND HIREDIS_LIB)
        target_compile_definitions(GildedRoseBackend PRIVATE USE_REDIS)
        target_include_directories(GildedRoseBackend PRIVATE ${HIREDIS_HEADER})
        target_link_libraries(GildedRoseBackend ${HIREDIS_LIB})
        message(STATUS "Redis cache backend ENABLED (hiredis: ${HIREDIS_LIB})")
    else()
        message(WARNING "USE_REDIS=ON but hiredis not found; using LRU fallback")
    endif()
endif()

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

# =====================================================
# Benchmark configuration with Google Benchmark
# =====================================================
option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)

if(BUILD_BENCHMARKS)
    include(FetchContent)
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googlebenchmark)

    file(GLOB_RECURSE BENCH_SOURCES "benchmarks/*.cpp")

    add_executable(GildedRoseBenchmarks ${BENCH_SOURCES} ${LIB_SOURCES})
    target_include_directories(GildedRoseBenchmarks PRIVATE src)
    target_link_libraries(GildedRoseBenchmarks benchmark::benchmark benchmark::benchmark_main sqlite3 pthread)
endif()
