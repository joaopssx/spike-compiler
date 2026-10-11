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
void run_ast_tests();

} // namespace spike_tests

#endif // SPIKE_TESTS_REGISTRY_HPP
