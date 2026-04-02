cmake_minimum_required(VERSION 3.16)

project(Spike LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(spike
    src/file_reader.cpp
    src/lexer.cpp
    src/main.cpp
    src/token.cpp
)

target_include_directories(spike PRIVATE include)

add_executable(spike_tests
    src/lexer.cpp
    src/token.cpp
    tests/lexer_tests.cpp
)

target_include_directories(spike_tests PRIVATE include)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(spike PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(spike_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME spike_lexer_tests COMMAND spike_tests)
