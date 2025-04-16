cmake_minimum_required(VERSION 3.10)


# Set the build type to Debug to enable -g (debug symbols)
set(CMAKE_BUILD_TYPE Debug)

# set debugging flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

# Project name
project(MultiProcessorCacheSim VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/cpu.cpp
    ${PROJECT_SOURCE_DIR}/src/cache.cpp
    ${PROJECT_SOURCE_DIR}/src/bus.cpp
    ${PROJECT_SOURCE_DIR}/src/trace.cpp
    ${PROJECT_SOURCE_DIR}/src/busmsg.cpp
    ${PROJECT_SOURCE_DIR}/src/cpumsg.cpp
    ${PROJECT_SOURCE_DIR}/src/globals.cpp
    ${PROJECT_SOURCE_DIR}/src/statistics.cpp
    ${PROJECT_SOURCE_DIR}/src/directory.cpp
    ${PROJECT_SOURCE_DIR}/src/dirmsg.cpp

)

# Add executable
add_executable(mpcsim ${SOURCES})

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "CXX_FLAGS (global): ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")