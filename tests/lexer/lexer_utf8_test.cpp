#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

using spike::TokenType;

std::string printed(const spike::Lexer& lex) {
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    lex.diagnostics().print_all();
    std::cerr.rdbuf(old);
    return sink.str();
}

void test_columns_count_characters() {
    const auto toks = spike_tests::tokens_of("\"Olá\" x");
    SPIKE_EXPECT(toks.size() == 2);
    SPIKE_EXPECT(toks[0].col == 1);
    SPIKE_EXPECT(toks[1].col == 7);
}

void test_token_length_in_characters() {
    const auto toks = spike_tests::tokens_of("abc <- \"Olá\" \"a\\\"b\" 3.25");
    SPIKE_EXPECT(toks.size() == 5);
    SPIKE_EXPECT(toks[0].length == 3);
    SPIKE_EXPECT(toks[1].length == 2);
    SPIKE_EXPECT(toks[2].length == 5);
    SPIKE_EXPECT(toks[3].length == 6);
    SPIKE_EXPECT(toks[4].length == 4);
}

void test_eof_has_no_length() {
    spike::Lexer lex("x", "<test>");
    const auto toks = lex.tokenize();
    SPIKE_EXPECT(toks.back().type == TokenType::END_OF_FILE);
    SPIKE_EXPECT(toks.back().length == 0);
    SPIKE_EXPECT(toks.back().col == 2);
}

void test_tab_counts_as_one_column() {
    const auto toks = spike_tests::tokens_of("\tx\t<-");
    SPIKE_EXPECT(toks[0].col == 2);
    SPIKE_EXPECT(toks[1].col == 4);
}

void test_accented_name_is_one_e001() {
    spike::Lexer lex("média <- 1", "<test>");
    const auto toks = lex.tokenize();
    SPIKE_EXPECT(lex.diagnostics().error_count() == 1);
    SPIKE_EXPECT(toks[0].type == TokenType::IDENTIFIER);
    SPIKE_EXPECT(toks[0].lexeme == "média");
    SPIKE_EXPECT(toks[0].length == 5);
    SPIKE_EXPECT(toks[1].col == 7);
    const std::string out = printed(lex);
    SPIKE_EXPECT(out.find("[E001]") != std::string::npos);
    SPIKE_EXPECT(out.find("'é'") != std::string::npos);
    SPIKE_EXPECT(out.find("<test>:1:2") != std::string::npos);
    SPIKE_EXPECT(out.find("use 'media' em vez de 'média'") != std::string::npos);
}

void test_name_starting_with_accent() {
    spike::Lexer lex("índice ação", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.diagnostics().error_count() == 2);
    const std::string out = printed(lex);
    SPIKE_EXPECT(out.find("use 'indice' em vez de 'índice'") != std::string::npos);
    SPIKE_EXPECT(out.find("use 'acao' em vez de 'ação'") != std::string::npos);
    SPIKE_EXPECT(out.find("<test>:1:9") != std::string::npos);
}

void test_other_unicode_symbol_is_one_e001() {
    spike::Lexer lex("x \xE2\x86\x90 1", "<test>");
    const auto toks = lex.tokenize();
    SPIKE_EXPECT(lex.diagnostics().error_count() == 1);
    SPIKE_EXPECT(toks[1].type == TokenType::UNKNOWN);
    SPIKE_EXPECT(toks[1].lexeme == "\xE2\x86\x90");
    SPIKE_EXPECT(toks[2].col == 5);
    SPIKE_EXPECT(printed(lex).find("caractere desconhecido '\xE2\x86\x90'") != std::string::npos);
}

void test_accents_in_strings_and_comments_are_fine() {
    spike::Lexer lex("// coment\xC3\xA1rio\n{ \xC3\xA7 }\n\"a\xC3\xA7\xC3\xA3o\"", "<test>");
    const auto toks = lex.tokenize();
    SPIKE_EXPECT(!lex.had_error());
    SPIKE_EXPECT(toks[0].lexeme == "a\xC3\xA7\xC3\xA3o");
}

void test_invalid_utf8_is_e005_and_stops() {
    const std::string source = std::string("x <- 1\ny \xC3\xA9") + static_cast<char>(0xFF) + " @";
    spike::Lexer lex(source, "<test>");
    const auto toks = lex.tokenize();
    SPIKE_EXPECT(lex.diagnostics().error_count() == 1);
    SPIKE_EXPECT(toks.size() == 1);
    const std::string out = printed(lex);
    SPIKE_EXPECT(out.find("[E005]") != std::string::npos);
    SPIKE_EXPECT(out.find("<test>:2:4") != std::string::npos);
}

void test_windows_1252_byte_is_e005() {
    const std::string source = std::string("escreval(\"Ol") + static_cast<char>(0xE1) + "\")";
    spike::Lexer lex(source, "<test>");
    lex.tokenize();
    SPIKE_EXPECT(printed(lex).find("<test>:1:13") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_lexer_utf8_tests() {
    test_columns_count_characters();
    test_token_length_in_characters();
    test_eof_has_no_length();
    test_tab_counts_as_one_column();
    test_accented_name_is_one_e001();
    test_name_starting_with_accent();
    test_other_unicode_symbol_is_one_e001();
    test_accents_in_strings_and_comments_are_fine();
    test_invalid_utf8_is_e005_and_stops();
    test_windows_1252_byte_is_e005();
}

} // namespace spike_tests
