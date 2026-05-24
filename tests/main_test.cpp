// Lightweight test runner for the Spike Compiler.
// Each test increments a failure counter on assertion error; the runner ends
// with "all tests passed" iff zero failures occurred, which is what CTest
// matches against.

#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_failures = 0;

#define SPIKE_EXPECT(cond)                                                     \
    do {                                                                       \
        if (!(cond)) {                                                         \
            ++g_failures;                                                      \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__               \
                      << ": " #cond << std::endl;                              \
        }                                                                      \
    } while (0)

// Helper: tokenize a string and return all tokens except the trailing EOF.
std::vector<spike::Token> tokens_of(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    std::vector<spike::Token> toks = lex.tokenize();
    if (!toks.empty() && toks.back().type == spike::TokenType::END_OF_FILE) {
        toks.pop_back();
    }
    return toks;
}

// ---- file_reader -----------------------------------------------------------

void test_read_file_missing() {
    spike::ReadResult r = spike::read_file("definitely_does_not_exist_xyz.por");
    SPIKE_EXPECT(r.ok == false);
    SPIKE_EXPECT(r.error.find("Arquivo não encontrado") != std::string::npos);
}

void test_read_file_ok() {
    const std::string path = "test_temp.por";
    const std::string payload = "algoritmo \"teste\"\n";
    {
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }
    spike::ReadResult r = spike::read_file(path);
    SPIKE_EXPECT(r.ok == true);
    SPIKE_EXPECT(r.content == payload);
    SPIKE_EXPECT(r.error.empty());
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

void test_read_file_wrong_extension_still_ok() {
    const std::string path = "test_temp.txt";
    const std::string payload = "qualquer coisa";
    {
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }
    spike::ReadResult r = spike::read_file(path);
    SPIKE_EXPECT(r.ok == true);
    SPIKE_EXPECT(r.content == payload);
    SPIKE_EXPECT(!r.error.empty());
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

// ---- token -----------------------------------------------------------------

void test_token_type_to_string_known_values() {
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::NUMBER) == "NUMBER");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::ALGORITMO) == "ALGORITMO");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::FIM_ENQUANTO) == "FIM_ENQUANTO");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::ARROW) == "ARROW");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::END_OF_FILE) == "END_OF_FILE");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::UNKNOWN) == "UNKNOWN");
}

void test_token_default_construction() {
    spike::Token t{spike::TokenType::IDENTIFIER, "contador", 3, 7};
    SPIKE_EXPECT(t.type == spike::TokenType::IDENTIFIER);
    SPIKE_EXPECT(t.lexeme == "contador");
    SPIKE_EXPECT(t.line == 3);
    SPIKE_EXPECT(t.col == 7);
}

// ---- lexer: keywords & identifiers (PROMPT 005) ---------------------------

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

// ---- lexer: numbers (PROMPT 006) ------------------------------------------

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

// ---- lexer: strings (PROMPT 006) ------------------------------------------

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
    SPIKE_EXPECT(toks[0].line == 4); // o '{' começou na 1, fechou na 3
}

void test_lexer_block_comment_unclosed_is_error() {
    spike::Lexer lex("{ nunca fecha", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
}

void test_lexer_unknown_char_is_error() {
    spike::Lexer lex("@", "<test>");
    auto toks = lex.tokenize();
    SPIKE_EXPECT(lex.had_error() == true);
    // toks: UNKNOWN, EOF
    SPIKE_EXPECT(toks.size() == 2);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::UNKNOWN);
}

} // namespace

int main() {
    test_read_file_missing();
    test_read_file_ok();
    test_read_file_wrong_extension_still_ok();

    test_token_type_to_string_known_values();
    test_token_default_construction();

    test_lexer_keywords_are_case_insensitive();
    test_lexer_identifier();

    test_lexer_integer_literal();
    test_lexer_real_literal();
    test_lexer_number_trailing_dot_is_error();

    test_lexer_string_simple();
    test_lexer_string_escape_sequences();
    test_lexer_string_unterminated_is_error();
    test_lexer_string_newline_is_error();

    test_lexer_single_char_operators();
    test_lexer_arrow();
    test_lexer_not_equal();
    test_lexer_less_eq_and_greater_eq();
    test_lexer_assign();
    test_lexer_colon_alone();
    test_lexer_less_and_greater_alone();
    test_lexer_line_comment_produces_no_token();
    test_lexer_block_comment_produces_no_token();
    test_lexer_block_comment_multiline();
    test_lexer_block_comment_unclosed_is_error();
    test_lexer_unknown_char_is_error();

    if (g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << g_failures << " test(s) failed" << std::endl;
    return 1;
}
