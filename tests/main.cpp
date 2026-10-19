// Lightweight test runner for the Spike Compiler.
// Each test increments a failure counter on assertion error; the runner ends
// with "all tests passed" iff zero failures occurred, which is what CTest
// matches against.

#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

struct Group {
    const char* name;
    void (*run)();
};

const std::vector<Group> kGroups = {
    {"file_reader", spike_tests::run_file_reader_tests},
    {"token", spike_tests::run_token_tests},
    {"lexer_keyword", spike_tests::run_lexer_keyword_tests},
    {"lexer_number", spike_tests::run_lexer_number_tests},
    {"lexer_string", spike_tests::run_lexer_string_tests},
    {"lexer_operator", spike_tests::run_lexer_operator_tests},
    {"lexer_comment", spike_tests::run_lexer_comment_tests},
    {"lexer_error", spike_tests::run_lexer_error_tests},
    {"diagnostics", spike_tests::run_diagnostics_tests},
    {"cli", spike_tests::run_cli_tests},
    {"console", spike_tests::run_console_tests},
    {"ast", spike_tests::run_ast_tests},
    {"ast_enum", spike_tests::run_ast_enum_tests},
    {"parser_program", spike_tests::run_parser_program_tests},
    {"parser_expr_literal", spike_tests::run_parser_expr_literal_tests},
    {"parser_expr_operator", spike_tests::run_parser_expr_operator_tests},
    {"parser_stmt_basic", spike_tests::run_parser_stmt_basic_tests},
    {"parser_stmt_if", spike_tests::run_parser_stmt_if_tests},
    {"parser_stmt_loop", spike_tests::run_parser_stmt_loop_tests},
    {"parser_expr_power", spike_tests::run_parser_expr_power_tests},
    {"parser_const", spike_tests::run_parser_const_tests},
    {"semantic_const", spike_tests::run_semantic_const_tests},
    {"codegen_const", spike_tests::run_codegen_const_tests},
    {"semantic_break", spike_tests::run_semantic_break_tests},
    {"codegen_break", spike_tests::run_codegen_break_tests},
    {"parser_escolha", spike_tests::run_parser_escolha_tests},
    {"semantic_escolha", spike_tests::run_semantic_escolha_tests},
    {"codegen_escolha", spike_tests::run_codegen_escolha_tests},
    {"parser_array", spike_tests::run_parser_array_tests},
    {"semantic_array", spike_tests::run_semantic_array_tests},
    {"codegen_array", spike_tests::run_codegen_array_tests},
    {"parser_procedure", spike_tests::run_parser_procedure_tests},
    {"semantic_procedure", spike_tests::run_semantic_procedure_tests},
    {"codegen_procedure", spike_tests::run_codegen_procedure_tests},
    {"parser_function", spike_tests::run_parser_function_tests},
    {"semantic_function", spike_tests::run_semantic_function_tests},
    {"codegen_function", spike_tests::run_codegen_function_tests},
    {"parser_name_position", spike_tests::run_parser_name_position_tests},
    {"symbol_table", spike_tests::run_symbol_table_tests},
    {"semantic_name", spike_tests::run_semantic_name_tests},
    {"semantic_operator", spike_tests::run_semantic_operator_tests},
    {"semantic_assign", spike_tests::run_semantic_assign_tests},
    {"semantic_control", spike_tests::run_semantic_control_tests},
    {"semantic_program", spike_tests::run_semantic_program_tests},
    {"runtime_text", spike_tests::run_runtime_text_tests},
    {"codegen_program", spike_tests::run_codegen_program_tests},
    {"codegen_expr", spike_tests::run_codegen_expr_tests},
    {"codegen_output", spike_tests::run_codegen_output_tests},
    {"codegen_control", spike_tests::run_codegen_control_tests},
    {"codegen_input", spike_tests::run_codegen_input_tests},
    {"codegen_compile", spike_tests::run_codegen_compile_tests},
    {"process", spike_tests::run_process_tests},
    {"cc_driver", spike_tests::run_cc_driver_tests},
    {"output_path", spike_tests::run_output_path_tests},
    {"ast_printer", spike_tests::run_ast_printer_tests},
};

bool selected(const std::string& name, const std::vector<std::string>& filters) {
    if (filters.empty()) return true;
    for (const std::string& filter : filters) {
        if (name.find(filter) != std::string::npos) return true;
    }
    return false;
}

} // namespace

int main(int argc, char** argv) {
    const std::vector<std::string> filters(argv + 1, argv + argc);
    int ran = 0;
    for (const Group& group : kGroups) {
        if (!selected(group.name, filters)) continue;
        group.run();
        ++ran;
    }
    if (ran == 0) {
        std::cerr << "no test group matches the filter" << std::endl;
        return 1;
    }

    if (spike_tests::g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << spike_tests::g_failures << " test(s) failed" << std::endl;
    return 1;
}
