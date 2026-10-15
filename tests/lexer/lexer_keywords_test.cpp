#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

namespace {

using spike_tests::tokens_of;

// ---- lexer: keywords & identifiers ----------------------------------------

void test_lexer_keywords_are_case_insensitive() {
    auto toks = tokens_of("Algoritmo SE entao");
    SPIKE_EXPECT(toks.size() == 3);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::ALGORITMO);
    SPIKE_EXPECT(toks[0].lexeme == "Algoritmo"); // preserva case original
    SPIKE_EXPECT(toks[1].type == spike::TokenType::SE);
    SPIKE_EXPECT(toks[2].type == spike::TokenType::ENTAO);
}

void test_lexer_identifier() {
    auto toks = tokens_of("contador _x x1");
    SPIKE_EXPECT(toks.size() == 3);
    for (const auto& t : toks) {
        SPIKE_EXPECT(t.type == spike::TokenType::IDENTIFIER);
    }
}

} // namespace

namespace spike_tests {

void run_lexer_keyword_tests() {
    test_lexer_keywords_are_case_insensitive();
    test_lexer_identifier();
}

} // namespace spike_tests
