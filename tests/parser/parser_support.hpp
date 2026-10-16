#ifndef SPIKE_TESTS_PARSER_SUPPORT_HPP
#define SPIKE_TESTS_PARSER_SUPPORT_HPP

#include "spike/ast.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace spike_tests {

// Helper: lex + parse `src` e retorna o Program junto com o parser pra
// inspecionar diagnosticos.
struct ParseResult {
    spike::Program program;
    bool had_error;
    std::string diag_output;
};

inline ParseResult parse_source(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    auto tokens = lex.tokenize();
    spike::Parser parser(std::move(tokens), "<test>", src);
    ParseResult r;
    r.program = parser.parse();
    r.had_error = parser.had_error();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    parser.diagnostics().print_all();
    std::cerr.rdbuf(old);
    r.diag_output = sink.str();
    return r;
}

// Helper: lex `src` e parseia uma unica expressao.
struct ExprResult {
    spike::ExprPtr expr;
    bool had_error;
};

inline ExprResult parse_expression(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    auto tokens = lex.tokenize();
    spike::Parser parser(std::move(tokens), "<test>", src);
    ExprResult r;
    r.expr = parser.parse_expr();
    r.had_error = parser.had_error();
    return r;
}

} // namespace spike_tests

#endif // SPIKE_TESTS_PARSER_SUPPORT_HPP
