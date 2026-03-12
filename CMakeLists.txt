cmake_minimum_required(VERSION 3.16)

project(spike VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(StandardProjectSettings)
include(CompilerWarnings)
include(Sanitizers)
include(SpikePackaging)

add_executable(spike
    src/main.cpp
    src/cli/options.cpp
    src/lexer/token_json.cpp
    src/core/source_manager.cpp
    src/core/diagnostics.cpp
    src/core/phase_profiler.cpp
    src/core/program_stats.cpp
    src/core/session.cpp
    src/core/version.cpp
    src/core/environment.cpp
    src/ast/ast_json.cpp
    src/ast/ast_graphviz.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ir/ir_builder.cpp
    src/ir/ir_printer.cpp
    src/passes/pass_manager.cpp
    src/passes/constant_folding_pass.cpp
    src/semantic.cpp
    src/codegen.cpp
    src/utils.cpp
)

target_include_directories(spike PRIVATE include)

spike_enable_warnings(spike)
spike_enable_sanitizers(spike)

spike_install_target(spike)

enable_testing()

add_test(NAME spike_tokens COMMAND spike tokens "${CMAKE_SOURCE_DIR}/examples/hello.por")
add_test(NAME spike_ast COMMAND spike ast "${CMAKE_SOURCE_DIR}/examples/if.por")
add_test(NAME spike_emit_c COMMAND spike emit-c "${CMAKE_SOURCE_DIR}/examples/loop.por")
add_test(NAME spike_check COMMAND spike check "${CMAKE_SOURCE_DIR}/examples/basic/hello.por")
add_test(NAME spike_doctor COMMAND spike doctor)
add_test(NAME spike_stats COMMAND spike stats "${CMAKE_SOURCE_DIR}/examples/basic/hello.por")
add_test(NAME spike_ast_json COMMAND spike ast "${CMAKE_SOURCE_DIR}/examples/control-flow/if.por" --json --stdout)
add_test(NAME spike_ir COMMAND spike ir "${CMAKE_SOURCE_DIR}/examples/basic/hello.por" --stdout)
