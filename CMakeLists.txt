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
    src/file_reader.cpp
    src/lexer.cpp
    src/token.cpp
    tests/lexer_tests.cpp
)

target_include_directories(spike_tests PRIVATE include)
target_compile_definitions(spike_tests PRIVATE SPIKE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(spike PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(spike_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME spike_lexer_tests COMMAND spike_tests)

add_test(NAME spike_cli_help COMMAND $<TARGET_FILE:spike> --help)
set_tests_properties(spike_cli_help PROPERTIES
    PASS_REGULAR_EXPRESSION "tokens <arquivo\\.por> \\[--verbose\\]"
)

add_test(NAME spike_cli_version COMMAND $<TARGET_FILE:spike> --version)
set_tests_properties(spike_cli_version PROPERTIES
    PASS_REGULAR_EXPRESSION "Spike 0\\.1\\.3"
)

add_test(
    NAME spike_cli_tokens_basic
    COMMAND $<TARGET_FILE:spike> tokens
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/valid/control_flow_case_insensitive.por"
)
set_tests_properties(spike_cli_tokens_basic PROPERTIES
    PASS_REGULAR_EXPRESSION "TOKEN_NOT_EQUAL"
)

add_test(
    NAME spike_cli_tokens_verbose
    COMMAND $<TARGET_FILE:spike> tokens
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/valid/control_flow_case_insensitive.por"
            --verbose
)
set_tests_properties(spike_cli_tokens_verbose PROPERTIES
    PASS_REGULAR_EXPRESSION "TOKEN_ALGORITMO lexeme=\"Algoritmo\" line=1 column=1"
)
