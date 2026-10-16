#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

namespace {

using spike_tests::tokens_of;

void test_lexer_line_comment_produces_no_token() {
    auto toks = tokens_of("// isso eh um comentario\n42");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::NUMBER);
    SPIKE_EXPECT(toks[0].lexeme == "42");
}

void test_lexer_block_comment_produces_no_token() {
    auto toks = tokens_of("{ comentario de bloco } 7");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::NUMBER);
    SPIKE_EXPECT(toks[0].lexeme == "7");
}

void test_lexer_block_comment_multiline() {
    auto toks = tokens_of("{ linha 1\nlinha 2\nlinha 3 }\nx");
    SPIKE_EXPECT(toks.size() == 1);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::IDENTIFIER);
    SPIKE_EXPECT(toks[0].line == 4); // o '{' comecou na 1, fechou na 3
}

void test_lexer_block_comment_unclosed_is_error() {
    spike::Lexer lex("{ nunca fecha", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
}

} // namespace

namespace spike_tests {

void run_lexer_comment_tests() {
    test_lexer_line_comment_produces_no_token();
    test_lexer_block_comment_produces_no_token();
    test_lexer_block_comment_multiline();
    test_lexer_block_comment_unclosed_is_error();
}

} // namespace spike_tests
