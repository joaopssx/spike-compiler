cmake_minimum_required(VERSION 3.16)

project(spike VERSION 0.1.0 LANGUAGES CXX)

add_executable(spike
    src/lexer.cpp
    src/main.cpp
    src/token.cpp
)

target_include_directories(spike PRIVATE src)

target_compile_features(spike PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(spike PRIVATE -Wall -Wextra -Wpedantic)
endif()
