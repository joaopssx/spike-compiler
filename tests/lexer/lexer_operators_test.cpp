#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

namespace {

using spike_tests::tokens_of;

// ---- lexer: operators, punctuation, comments (PROMPT 007) -----------------

void test_lexer_single_char_operators() {
    auto toks = tokens_of("+ - * / ( ) , . =");
    SPIKE_EXPECT(toks.size() == 9);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::PLUS);
    SPIKE_EXPECT(toks[1].type == spike::TokenType::MINUS);
    SPIKE_EXPECT(toks[2].type == spike::TokenType::STAR);
    SPIKE_EXPECT(toks[3].type == spike::TokenType::SLASH);
    SPIKE_EXPECT(toks[4].type == spike::TokenType::LPAREN);
    SPIKE_EXPECT(toks[5].type == spike::TokenType::RPAREN);
    SPIKE_EXPECT(toks[6].type == spike::TokenType::COMMA);
    SPIKE_EXPECT(toks[7].type == spike::TokenType::DOT);
    SPIKE_EXPECT(toks[8].type == spike::TokenType::EQUAL);
}

void test_lexer_arrow() {
    auto toks = tokens_of("<-");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::ARROW);
    SPIKE_EXPECT(toks[0].lexeme == "<-");
}

void test_lexer_not_equal() {
    auto toks = tokens_of("<>");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::NOT_EQUAL);
}

void test_lexer_less_eq_and_greater_eq() {
    auto toks = tokens_of("<= >=");
    SPIKE_EXPECT(toks.size() == 2);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::LESS_EQ);
    SPIKE_EXPECT(toks[1].type == spike::TokenType::GREATER_EQ);
}

void test_lexer_assign() {
    auto toks = tokens_of(":=");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::ASSIGN);
}

void test_lexer_colon_alone() {
    auto toks = tokens_of(":");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::COLON);
}

void test_lexer_less_and_greater_alone() {
    auto toks = tokens_of("< >");
    SPIKE_EXPECT(toks.size() == 2);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::LESS);
    SPIKE_EXPECT(toks[1].type == spike::TokenType::GREATER);
}

} // namespace

namespace spike_tests {

void run_lexer_operator_tests() {
    test_lexer_single_char_operators();
    test_lexer_arrow();
    test_lexer_not_equal();
    test_lexer_less_eq_and_greater_eq();
    test_lexer_assign();
    test_lexer_colon_alone();
    test_lexer_less_and_greater_alone();
}

} // namespace spike_tests
