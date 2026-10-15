// Lightweight test runner for the Spike Compiler.
// Each test increments a failure counter on assertion error; the runner ends
// with "all tests passed" iff zero failures occurred, which is what CTest
// matches against.

#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>

int main() {
    spike_tests::run_file_reader_tests();
    spike_tests::run_token_tests();
    spike_tests::run_lexer_keyword_tests();
    spike_tests::run_lexer_number_tests();
    spike_tests::run_lexer_string_tests();
    spike_tests::run_lexer_operator_tests();
    spike_tests::run_lexer_comment_tests();
    spike_tests::run_lexer_error_tests();
    spike_tests::run_diagnostics_tests();
    spike_tests::run_cli_tests();
    spike_tests::run_console_tests();
    spike_tests::run_ast_tests();
    spike_tests::run_ast_enum_tests();
    spike_tests::run_parser_program_tests();
    spike_tests::run_parser_expr_literal_tests();
    spike_tests::run_parser_expr_operator_tests();
    spike_tests::run_parser_stmt_basic_tests();
    spike_tests::run_parser_stmt_if_tests();
    spike_tests::run_parser_stmt_loop_tests();
    spike_tests::run_parser_name_position_tests();
    spike_tests::run_symbol_table_tests();
    spike_tests::run_semantic_name_tests();
    spike_tests::run_semantic_operator_tests();
    spike_tests::run_semantic_assign_tests();
    spike_tests::run_semantic_control_tests();
    spike_tests::run_semantic_program_tests();
    spike_tests::run_runtime_text_tests();
    spike_tests::run_codegen_program_tests();
    spike_tests::run_codegen_expr_tests();
    spike_tests::run_codegen_compile_tests();

    if (spike_tests::g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << spike_tests::g_failures << " test(s) failed" << std::endl;
    return 1;
}
