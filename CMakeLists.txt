cmake_minimum_required(VERSION 3.16)

project(spike VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(spike
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic.cpp
    src/codegen.cpp
    src/utils.cpp
)

target_include_directories(spike PRIVATE include)

if(MSVC)
    target_compile_options(spike PRIVATE /W4)
else()
    target_compile_options(spike PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS spike RUNTIME DESTINATION bin)

enable_testing()

add_test(NAME spike_tokens COMMAND spike tokens "${CMAKE_SOURCE_DIR}/examples/hello.por")
add_test(NAME spike_ast COMMAND spike ast "${CMAKE_SOURCE_DIR}/examples/if.por")
add_test(NAME spike_emit_c COMMAND spike emit-c "${CMAKE_SOURCE_DIR}/examples/loop.por")
