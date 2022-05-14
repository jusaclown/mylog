# 指定cmake的最小版本
cmake_minimum_required(VERSION 3.10...3.23)

# ---------------------------------------------------------------------------------------
# Start mylog project
# ---------------------------------------------------------------------------------------
include(cmake/utils.cmake)

PROJECT(mylog VERSION 0.0.1 LANGUAGES CXX)
message(STATUS "Build mylog")


# ---------------------------------------------------------------------------------------
# Set default build to Debug
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# ---------------------------------------------------------------------------------------
# Set MYLOG_MASTER_PROJECT to ON if we are building mylog
# ---------------------------------------------------------------------------------------
option(MYLOG_BUILD_ALL "Build all artifacts" OFF)

# example options
option(MYLOG_BUILD_EXAMPLE "Build example" ON)

# test options
option(MYLOG_BUILD_TESTS "Build tests" ON)

# bench options
option(MYLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" ON)

#sanitizer options
option(MYLOG_SANITIZE_ADDRESS "Enable address sanitizer in tests" ON)

#warning options
option(MYLOG_BUILD_WARNINGS "Enable compiler warnings" ON)

find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# ---------------------------------------------------------------------------------------
# Static/Shared library
# ---------------------------------------------------------------------------------------
file(GLOB_RECURSE MYLOG_SRCS "${PROJECT_SOURCE_DIR}/log/*.cc")
file(GLOB_RECURSE MYLOG_HEADERS "${PROJECT_SOURCE_DIR}/log/*.h")

add_library(mylog STATIC ${MYLOG_SRCS} ${MYLOG_HEADERS})
add_library(mylog::mylog ALIAS mylog)

target_include_directories(mylog PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(mylog PUBLIC Threads::Threads)
target_link_libraries(mylog PUBLIC fmt::fmt)

mylog_enable_warnings(mylog)

set_target_properties(mylog PROPERTIES DEBUG_POSTFIX d)

# ---------------------------------------------------------------------------------------
# Build binaries
# ---------------------------------------------------------------------------------------
if (MYLOG_BUILD_EXAMPLE OR MYLOG_BUILD_ALL)
    message(STATUS "Generating example(s)")
    add_subdirectory(example)
endif()

if (MYLOG_BUILD_TESTS OR MYLOG_BUILD_ALL)
    message(STATUS "Generating tests")
    enable_testing()
    add_subdirectory(tests)
endif()

if (MYLOG_BUILD_BENCH OR MYLOG_BUILD_ALL)
    message(STATUS "Generating benchmarks")
    add_subdirectory(bench)
endif()