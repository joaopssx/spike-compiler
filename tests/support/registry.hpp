#ifndef SPIKE_TESTS_REGISTRY_HPP
#define SPIKE_TESTS_REGISTRY_HPP

namespace spike_tests {

void run_file_reader_tests();
void run_token_tests();
void run_lexer_keyword_tests();
void run_lexer_number_tests();
void run_lexer_string_tests();
void run_lexer_operator_tests();
void run_lexer_comment_tests();
void run_lexer_error_tests();
void run_diagnostics_tests();
void run_cli_tests();
void run_console_tests();
void run_ast_tests();
void run_ast_enum_tests();
void run_parser_program_tests();
void run_parser_expr_literal_tests();
void run_parser_expr_operator_tests();
void run_parser_stmt_basic_tests();
void run_parser_stmt_if_tests();
void run_parser_stmt_loop_tests();
void run_parser_name_position_tests();
void run_symbol_table_tests();
void run_semantic_name_tests();
void run_semantic_operator_tests();
void run_semantic_assign_tests();
void run_semantic_control_tests();
void run_semantic_program_tests();
void run_runtime_text_tests();
void run_codegen_program_tests();
void run_codegen_expr_tests();
void run_codegen_output_tests();
void run_codegen_compile_tests();
void run_process_tests();
void run_cc_driver_tests();

} // namespace spike_tests

#endif // SPIKE_TESTS_REGISTRY_HPP
