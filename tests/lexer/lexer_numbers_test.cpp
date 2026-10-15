#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

namespace {

using spike_tests::tokens_of;

// ---- lexer: numbers -------------------------------------------------------

void test_lexer_integer_literal() {
    auto toks = tokens_of("42");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::NUMBER);
    SPIKE_EXPECT(toks[0].lexeme == "42");
}

void test_lexer_real_literal() {
    auto toks = tokens_of("3.14");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::NUMBER);
    SPIKE_EXPECT(toks[0].lexeme == "3.14");
}

void test_lexer_number_trailing_dot_is_error() {
    spike::Lexer lex("3.", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
}

} // namespace

namespace spike_tests {

void run_lexer_number_tests() {
    test_lexer_integer_literal();
    test_lexer_real_literal();
    test_lexer_number_trailing_dot_is_error();
}

} // namespace spike_tests
