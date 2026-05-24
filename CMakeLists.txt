cmake_minimum_required(VERSION 3.16)

project(Spike LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Shared warning flags ----------------------------------------------------
add_library(spike_warnings INTERFACE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(spike_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# ---- Core library ------------------------------------------------------------
add_library(spike_core STATIC
    src/file_reader.cpp
    src/token.cpp
    src/lexer.cpp
)
target_include_directories(spike_core PUBLIC include)
target_compile_definitions(spike_core PUBLIC SPIKE_VERSION="0.1.0")
target_link_libraries(spike_core PRIVATE spike_warnings)

# ---- CLI executable ----------------------------------------------------------
add_executable(spike src/main.cpp)
target_link_libraries(spike PRIVATE spike_core spike_warnings)

# ---- Tests -------------------------------------------------------------------
enable_testing()

add_executable(spike_tests tests/main_test.cpp)
target_link_libraries(spike_tests PRIVATE spike_core spike_warnings)

add_test(NAME spike_version_check COMMAND spike --version)
set_tests_properties(spike_version_check PROPERTIES
    PASS_REGULAR_EXPRESSION "0\\.1"
)

add_test(NAME spike_unit_tests COMMAND spike_tests)
set_tests_properties(spike_unit_tests PROPERTIES
    PASS_REGULAR_EXPRESSION "all tests passed"
)
