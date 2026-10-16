cmake_minimum_required(VERSION 3.16)

project(Spike VERSION 0.2.0 LANGUAGES CXX)

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
# Headers sao auto-descobertos pelo dependency scanner do CMake, entao
# tecnicamente nao precisariam estar listados aqui. Mantemos `ast.hpp` (e os
# outros headers publicos) listados pra aparecer na lista de fontes do IDE e
# pra qualquer ferramenta que itere sobre as sources do target encontrar.
set(SPIKE_AST_SOURCES
    src/ast/ast_enums.cpp
    src/ast/ast_printer.cpp
)
set(SPIKE_IO_SOURCES
    src/io/file_reader.cpp
)
set(SPIKE_LEXER_SOURCES
    src/lexer/token.cpp
    src/lexer/lexer_core.cpp
    src/lexer/lexer_scan.cpp
    src/lexer/lexer_hints.hpp
)
set(SPIKE_DIAGNOSTICS_SOURCES
    src/diagnostics/diagnostics.cpp
)
set(SPIKE_CLI_SOURCES
    src/cli/cli.cpp
    src/cli/console.cpp
)
set(SPIKE_PARSER_SOURCES
    src/parser/parser_core.cpp
    src/parser/parser_program.cpp
    src/parser/parser_stmt.cpp
    src/parser/parser_control.cpp
    src/parser/parser_expr.cpp
)
set(SPIKE_SEMANTIC_SOURCES
    src/semantic/symbol_table.cpp
    src/semantic/semantic_core.cpp
    src/semantic/semantic_expr.cpp
    src/semantic/semantic_stmt.cpp
    src/semantic/semantic_types.cpp
    src/semantic/semantic_control.cpp
    src/semantic/semantic_const.cpp
    src/semantic/const_eval.cpp
    src/semantic/semantic_internal.hpp
)
set(SPIKE_PROCESS_SOURCES
    src/process/executable_search.cpp
    src/process/process_posix.cpp
    src/process/process_windows.cpp
    src/process/temp_dir.cpp
    src/process/utf8_path.hpp
)
set(SPIKE_DRIVER_SOURCES
    src/driver/cc_driver.cpp
    src/driver/compile_pipeline.cpp
    src/driver/output_paths.cpp
    src/driver/pipeline_front.cpp
    src/driver/pipeline_report.cpp
    src/driver/pipeline_internal.hpp
)
set(SPIKE_CODEGEN_SOURCES
    src/codegen/codegen_core.cpp
    src/codegen/codegen_expr.cpp
    src/codegen/codegen_stmt.cpp
    src/codegen/codegen_control.cpp
    src/codegen/codegen_names.cpp
    src/codegen/codegen_internal.hpp
)
set(SPIKE_RUNTIME_TEXT_CPP "${CMAKE_BINARY_DIR}/generated/runtime_text.cpp")
add_custom_command(
    OUTPUT "${SPIKE_RUNTIME_TEXT_CPP}"
    COMMAND ${CMAKE_COMMAND}
        "-DINPUTS=${CMAKE_SOURCE_DIR}/runtime/spike_runtime_core.h|${CMAKE_SOURCE_DIR}/runtime/spike_runtime_io.h"
        "-DOUTPUT=${SPIKE_RUNTIME_TEXT_CPP}"
        -P "${CMAKE_SOURCE_DIR}/cmake/embed_runtime.cmake"
    DEPENDS
        "${CMAKE_SOURCE_DIR}/runtime/spike_runtime_core.h"
        "${CMAKE_SOURCE_DIR}/runtime/spike_runtime_io.h"
        "${CMAKE_SOURCE_DIR}/cmake/embed_runtime.cmake"
    COMMENT "Embedding the C runtime"
    VERBATIM
)
set(SPIKE_RUNTIME_SOURCES
    "${SPIKE_RUNTIME_TEXT_CPP}"
    runtime/spike_runtime.h
    runtime/spike_runtime_core.h
    runtime/spike_runtime_io.h
)
set(SPIKE_PUBLIC_HEADERS
    include/spike/ast.hpp
    include/spike/ast_enums.hpp
    include/spike/ast_printer.hpp
    include/spike/cc_driver.hpp
    include/spike/cli.hpp
    include/spike/codegen.hpp
    include/spike/compile_pipeline.hpp
    include/spike/const_value.hpp
    include/spike/colors.hpp
    include/spike/console.hpp
    include/spike/diagnostics.hpp
    include/spike/file_reader.hpp
    include/spike/lexer.hpp
    include/spike/parser.hpp
    include/spike/process.hpp
    include/spike/runtime_text.hpp
    include/spike/semantic.hpp
    include/spike/symbol_table.hpp
    include/spike/temp_dir.hpp
    include/spike/token.hpp
)

add_library(spike_core STATIC
    ${SPIKE_AST_SOURCES}
    ${SPIKE_IO_SOURCES}
    ${SPIKE_LEXER_SOURCES}
    ${SPIKE_DIAGNOSTICS_SOURCES}
    ${SPIKE_CLI_SOURCES}
    ${SPIKE_PARSER_SOURCES}
    ${SPIKE_SEMANTIC_SOURCES}
    ${SPIKE_RUNTIME_SOURCES}
    ${SPIKE_CODEGEN_SOURCES}
    ${SPIKE_PROCESS_SOURCES}
    ${SPIKE_DRIVER_SOURCES}
    ${SPIKE_PUBLIC_HEADERS}
)
target_include_directories(spike_core PUBLIC include PRIVATE src)
target_compile_definitions(spike_core PUBLIC SPIKE_VERSION="${PROJECT_VERSION}")
target_link_libraries(spike_core PRIVATE spike_warnings)

# ---- CLI executable ----------------------------------------------------------
add_executable(spike src/main.cpp)
target_link_libraries(spike PRIVATE spike_core spike_warnings)

# ---- Tests -------------------------------------------------------------------
enable_testing()

add_executable(spike_tests
    tests/main.cpp
    tests/ast/ast_test.cpp
    tests/ast/ast_enums_test.cpp
    tests/ast/ast_printer_test.cpp
    tests/cli/cli_test.cpp
    tests/cli/console_test.cpp
    tests/codegen/codegen_compile_test.cpp
    tests/codegen/codegen_control_test.cpp
    tests/codegen/codegen_expr_test.cpp
    tests/codegen/codegen_input_test.cpp
    tests/codegen/codegen_output_test.cpp
    tests/codegen/codegen_program_test.cpp
    tests/diagnostics/diagnostics_test.cpp
    tests/driver/output_paths_test.cpp
    tests/file_reader/file_reader_test.cpp
    tests/lexer/lexer_comments_test.cpp
    tests/lexer/lexer_errors_test.cpp
    tests/lexer/lexer_keywords_test.cpp
    tests/lexer/lexer_numbers_test.cpp
    tests/lexer/lexer_operators_test.cpp
    tests/lexer/lexer_strings_test.cpp
    tests/parser/expr_literals_test.cpp
    tests/parser/expr_operators_test.cpp
    tests/parser/expr_power_test.cpp
    tests/parser/const_section_test.cpp
    tests/semantic/semantic_const_test.cpp
    tests/codegen/codegen_const_test.cpp
    tests/parser/name_positions_test.cpp
    tests/parser/program_test.cpp
    tests/parser/stmt_basic_test.cpp
    tests/parser/stmt_if_test.cpp
    tests/parser/stmt_loops_test.cpp
    tests/process/cc_driver_test.cpp
    tests/process/process_test.cpp
    tests/semantic/semantic_assign_test.cpp
    tests/semantic/semantic_control_test.cpp
    tests/semantic/semantic_names_test.cpp
    tests/semantic/semantic_operators_test.cpp
    tests/semantic/semantic_programs_test.cpp
    tests/semantic/symbol_table_test.cpp
    tests/runtime/runtime_text_test.cpp
    tests/token/token_test.cpp
)
target_include_directories(spike_tests PRIVATE tests src)
target_compile_definitions(spike_tests PRIVATE
    SPIKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    SPIKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
)
target_link_libraries(spike_tests PRIVATE spike_core spike_warnings)

string(REPLACE "." "\\." SPIKE_VERSION_REGEX "${PROJECT_VERSION}")
add_test(NAME spike_version_check COMMAND spike --version)
set_tests_properties(spike_version_check PROPERTIES
    PASS_REGULAR_EXPRESSION "${SPIKE_VERSION_REGEX}"
)

add_test(NAME spike_unit_tests COMMAND spike_tests)
set_tests_properties(spike_unit_tests PROPERTIES
    PASS_REGULAR_EXPRESSION "all tests passed"
)

include(tests/programs.cmake)
include(tests/runtime.cmake)
include(tests/process.cmake)
include(tests/compile.cmake)
include(tests/e2e.cmake)
