#ifndef SPIKE_TESTS_SEMANTIC_SUPPORT_HPP
#define SPIKE_TESTS_SEMANTIC_SUPPORT_HPP

#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/semantic.hpp"
#include "spike/symbol_table.hpp"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

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

} // namespace spike_tests

#endif // SPIKE_TESTS_SEMANTIC_SUPPORT_HPP
