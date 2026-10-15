#ifndef SPIKE_TESTS_SEMANTIC_SUPPORT_HPP
#define SPIKE_TESTS_SEMANTIC_SUPPORT_HPP

#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/semantic.hpp"
#include "spike/symbol_table.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

namespace spike_tests {

struct SemanticResult {
    bool parse_error = false;
    bool had_error = false;
    std::string diag_output;
    spike::SymbolTable symbols;
};

inline SemanticResult analyze_source(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    spike::Parser parser(lex.tokenize(), "<test>", src);
    const spike::Program program = parser.parse();

    spike::SemanticAnalyzer analyzer(program, "<test>", src);
    analyzer.analyze();

    SemanticResult r;
    r.parse_error = parser.had_error();
    r.had_error = analyzer.had_error();
    r.symbols = analyzer.symbols();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    analyzer.diagnostics().print_all();
    std::cerr.rdbuf(old);
    r.diag_output = sink.str();
    return r;
}

inline std::size_t count_occurrences(const std::string& text, const std::string& needle) {
    std::size_t count = 0;
    for (std::size_t pos = text.find(needle); pos != std::string::npos;
         pos = text.find(needle, pos + needle.size())) {
        ++count;
    }
    return count;
}

inline std::string program_with(const std::string& vars, const std::string& body) {
    return "algoritmo \"t\"\nvar\n" + vars + "inicio\n" + body + "fimalgoritmo\n";
}

struct TypedProgram {
    spike::Program program;
    std::unique_ptr<spike::SemanticAnalyzer> analyzer;
    std::string diag_output;
};

inline std::unique_ptr<TypedProgram> analyze_typed(const std::string& src) {
    auto typed = std::make_unique<TypedProgram>();
    spike::Lexer lex(src, "<test>");
    spike::Parser parser(lex.tokenize(), "<test>", src);
    typed->program = parser.parse();
    typed->analyzer = std::make_unique<spike::SemanticAnalyzer>(typed->program, "<test>", src);
    typed->analyzer->analyze();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    typed->analyzer->diagnostics().print_all();
    std::cerr.rdbuf(old);
    typed->diag_output = sink.str();
    return typed;
}

inline const char* kTypedVars =
    "  i, j : inteiro\n  r : real\n  t, u : caractere\n  b, c : logico\n";

struct ExprCheck {
    spike::TypeKind type = spike::TypeKind::None;
    std::string diag_output;
};

inline ExprCheck check_expression(const std::string& expr_text) {
    auto typed = analyze_typed(program_with(kTypedVars, "  escreval(" + expr_text + ")\n"));
    ExprCheck result;
    result.diag_output = typed->diag_output;
    if (typed->program.body.empty()) return result;
    const auto* escreva = std::get_if<spike::EscrevaStmt>(&typed->program.body[0]->node);
    if (!escreva || escreva->args.empty()) return result;
    const auto& types = typed->analyzer->expr_types();
    const auto it = types.find(escreva->args[0].get());
    if (it != types.end()) result.type = it->second;
    return result;
}

} // namespace spike_tests

#endif // SPIKE_TESTS_SEMANTIC_SUPPORT_HPP
