cmake_minimum_required(VERSION 3.16)

project(Spike LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(spike
    src/main.cpp
    src/file_reader.cpp
    src/token.cpp
)

target_include_directories(spike PRIVATE include)

target_compile_definitions(spike PRIVATE SPIKE_VERSION="0.1.0")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(spike PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(spike_tests
    tests/main_test.cpp
    src/file_reader.cpp
    src/token.cpp
)
target_include_directories(spike_tests PRIVATE include)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(spike_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME spike_version_check COMMAND spike --version)
set_tests_properties(spike_version_check PROPERTIES
    PASS_REGULAR_EXPRESSION "0\\.1"
)

add_test(NAME spike_unit_tests COMMAND spike_tests)
set_tests_properties(spike_unit_tests PROPERTIES
    PASS_REGULAR_EXPRESSION "all tests passed"
)
