#ifndef SPIKE_TESTS_CODEGEN_SUPPORT_HPP
#define SPIKE_TESTS_CODEGEN_SUPPORT_HPP

#include "spike/codegen.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/semantic.hpp"

#include <string>

namespace spike_tests {

struct GeneratedC {
    bool front_end_ok = false;
    spike::CodegenResult result;
};

inline GeneratedC generate_from_source(const std::string& src,
                                       const std::string& name = "teste.por") {
    GeneratedC out;
    spike::Lexer lex(src, name);
    spike::Parser parser(lex.tokenize(), name, src);
    const spike::Program program = parser.parse();
    spike::SemanticAnalyzer analyzer(program, name, src);
    analyzer.analyze();
    out.front_end_ok = !lex.had_error() && !parser.had_error() && !analyzer.had_error();
    if (out.front_end_ok) out.result = spike::generate_c(program, analyzer.expr_types(), name);
    return out;
}

inline const char* kCodegenVars =
    "  a, i : inteiro\n  r : real\n  t, u : caractere\n  b, c : logico\n";

inline std::string generate_body(const std::string& body) {
    const std::string src = "algoritmo \"t\"\nvar\n" + std::string(kCodegenVars) + "inicio\n" +
                            body + "fimalgoritmo\n";
    return generate_from_source(src).result.c_source;
}

inline bool contains(const std::string& text, const std::string& piece) {
    return text.find(piece) != std::string::npos;
}

} // namespace spike_tests

#endif // SPIKE_TESTS_CODEGEN_SUPPORT_HPP
