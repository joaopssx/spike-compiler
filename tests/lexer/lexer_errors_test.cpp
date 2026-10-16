#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

void test_lexer_unknown_char_is_error() {
    spike::Lexer lex("@", "<test>");
    auto toks = lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
    SPIKE_EXPECT(lex.diagnostics().error_count() == 1);
    // toks: UNKNOWN, EOF
    SPIKE_EXPECT(toks.size() == 2);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::UNKNOWN);
}

// Helper para checar que um erro do lexer foi roteado via Diagnostics com o
// codigo esperado.
bool lexer_diag_contains_code(const spike::Lexer& lex, const std::string& code) {
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    lex.diagnostics().print_all();
    std::cerr.rdbuf(old);
    return sink.str().find(code) != std::string::npos;
}

// ---- lexer <-> diagnostics integration --------------------------------------

void test_lexer_diag_codes_are_wired() {
    {
        spike::Lexer lex("@", "<test>");
        lex.tokenize();
        SPIKE_EXPECT(lexer_diag_contains_code(lex, "E001"));
    }
    {
        spike::Lexer lex("\"unclosed", "<test>");
        lex.tokenize();
        SPIKE_EXPECT(lexer_diag_contains_code(lex, "E002"));
    }
    {
        spike::Lexer lex("3.", "<test>");
        lex.tokenize();
        SPIKE_EXPECT(lexer_diag_contains_code(lex, "E003"));
    }
    {
        spike::Lexer lex("{ never closed", "<test>");
        lex.tokenize();
        SPIKE_EXPECT(lexer_diag_contains_code(lex, "E004"));
    }
}

void test_lexer_diagnostics_carries_hints() {
    spike::Lexer lex("@", "<test>");
    lex.tokenize();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    lex.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("dica:") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_lexer_error_tests() {
    test_lexer_unknown_char_is_error();
    test_lexer_diag_codes_are_wired();
    test_lexer_diagnostics_carries_hints();
}

} // namespace spike_tests
