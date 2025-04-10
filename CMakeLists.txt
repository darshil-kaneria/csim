cmake_minimum_required(VERSION 3.10)

# Project name
project(MultiProcessorCacheSim VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/cache.cpp
    ${PROJECT_SOURCE_DIR}/src/cpu.cpp
    ${PROJECT_SOURCE_DIR}/src/intercon.cpp
    ${PROJECT_SOURCE_DIR}/src/memreq.cpp
    ${PROJECT_SOURCE_DIR}/src/trace.cpp
)

# Add executable
add_executable(mpcsim ${SOURCES})