#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

namespace {

using spike_tests::tokens_of;

// ---- lexer: strings -------------------------------------------------------

void test_lexer_string_simple() {
    auto toks = tokens_of("\"ola mundo\"");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::STRING);
    SPIKE_EXPECT(toks[0].lexeme == "ola mundo");
}

void test_lexer_string_escape_sequences() {
    // Source contains backslash+n, backslash+t, backslash+", backslash+\.
    auto toks = tokens_of("\"linha\\nseguinte\\ttab\\\"aspa\\\\barra\"");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::STRING);
    SPIKE_EXPECT(toks[0].lexeme == "linha\nseguinte\ttab\"aspa\\barra");
}

void test_lexer_string_unterminated_is_error() {
    spike::Lexer lex("\"nao fechada", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
}

void test_lexer_string_newline_is_error() {
    spike::Lexer lex("\"quebra\naqui\"", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
}

} // namespace

namespace spike_tests {

void run_lexer_string_tests() {
    test_lexer_string_simple();
    test_lexer_string_escape_sequences();
    test_lexer_string_unterminated_is_error();
    test_lexer_string_newline_is_error();
}

} // namespace spike_tests
