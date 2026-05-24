cmake_minimum_required(VERSION 3.16)

project(Spike LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(spike src/main.cpp)

target_include_directories(spike PRIVATE include)

target_compile_definitions(spike PRIVATE SPIKE_VERSION="0.1.0")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(spike PRIVATE -Wall -Wextra -Wpedantic)
endif()
