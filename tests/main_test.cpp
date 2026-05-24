// Lightweight test runner for the Spike Compiler.
// Each test increments a failure counter on assertion error; the runner ends
// with "all tests passed" iff zero failures occurred, which is what CTest
// matches against.

#include "spike/cli.hpp"
#include "spike/diagnostics.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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
    SPIKE_EXPECT(lex.diagnostics().error_count() == 1);
    // toks: UNKNOWN, EOF
    SPIKE_EXPECT(toks.size() == 2);
    SPIKE_EXPECT(toks[0].type == spike::TokenType::UNKNOWN);
}

// Helper para checar que um erro do lexer foi roteado via Diagnostics com o
// código esperado.
bool lexer_diag_contains_code(const spike::Lexer& lex, const std::string& code) {
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    lex.diagnostics().print_all();
    std::cerr.rdbuf(old);
    return sink.str().find(code) != std::string::npos;
}

// ---- lexer ↔ diagnostics integration (PROMPT 010) -------------------------

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

// ---- diagnostics (PROMPT 009) ---------------------------------------------

void test_diag_empty_starts_clean() {
    spike::Diagnostics d("foo.por", "linha 1\nlinha 2\n");
    SPIKE_EXPECT(d.has_errors() == false);
    SPIKE_EXPECT(d.error_count() == 0);
    SPIKE_EXPECT(d.warning_count() == 0);
}

void test_diag_error_counts() {
    spike::Diagnostics d("foo.por", "a\nb\nc\n");
    d.error("E001", "primeiro", 1, 1);
    d.error("E002", "segundo", 2, 1);
    d.warning("W001", "aviso", 3, 1);
    SPIKE_EXPECT(d.has_errors() == true);
    SPIKE_EXPECT(d.error_count() == 2);
    SPIKE_EXPECT(d.warning_count() == 1);
}

void test_diag_warning_only_does_not_set_has_errors() {
    spike::Diagnostics d("foo.por", "x\n");
    d.warning("W001", "so um aviso", 1, 1);
    SPIKE_EXPECT(d.has_errors() == false);
    SPIKE_EXPECT(d.warning_count() == 1);
}

void test_diag_handles_out_of_range_line() {
    // Linha 99 não existe — não deve crashar, source_line vira string vazia.
    spike::Diagnostics d("foo.por", "uma linha so\n");
    d.error("E999", "linha inexistente", 99, 1);
    SPIKE_EXPECT(d.error_count() == 1);
    // Smoke test: print_all não deve crashar nem com linha fora do range.
    // (Não verificamos saída — só que executa.)
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(!sink.str().empty());
}

void test_diag_print_contains_key_pieces() {
    spike::Diagnostics d("calc.por", "x <- 1\ny <- 2\nz <- 3\n");
    d.error("E042", "exemplo de erro", 2, 3, "use uma variavel valida");
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    const std::string out = sink.str();
    SPIKE_EXPECT(out.find("erro[E042]:") != std::string::npos);
    SPIKE_EXPECT(out.find("exemplo de erro") != std::string::npos);
    SPIKE_EXPECT(out.find("calc.por:2:3") != std::string::npos);
    SPIKE_EXPECT(out.find("y <- 2") != std::string::npos);          // a linha do erro
    SPIKE_EXPECT(out.find("x <- 1") != std::string::npos);          // contexto anterior
    SPIKE_EXPECT(out.find("z <- 3") != std::string::npos);          // contexto seguinte
    SPIKE_EXPECT(out.find("^") != std::string::npos);
    SPIKE_EXPECT(out.find("dica:") != std::string::npos);
    SPIKE_EXPECT(out.find("use uma variavel valida") != std::string::npos);
}

void test_diag_no_hint_omits_dica_line() {
    spike::Diagnostics d("a.por", "x\n");
    d.error("E001", "sem dica", 1, 1); // sem hint
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("dica:") == std::string::npos);
}

// ---- cli (PROMPT 011) ------------------------------------------------------

// Helper para chamar parse_args com um vetor de strings (em vez de argv).
// const_cast é seguro aqui porque parse_args só lê argv.
spike::CliArgs parse(const std::vector<std::string>& argv_in) {
    std::vector<char*> argv;
    argv.reserve(argv_in.size());
    for (const auto& s : argv_in) {
        argv.push_back(const_cast<char*>(s.c_str()));
    }
    return spike::parse_args(static_cast<int>(argv.size()), argv.data());
}

void test_cli_no_args_is_help() {
    auto a = parse({"spike"});
    SPIKE_EXPECT(a.command == spike::Command::HELP);
    SPIKE_EXPECT(a.parse_error == false);
}

void test_cli_help_flag() {
    SPIKE_EXPECT(parse({"spike", "--help"}).command == spike::Command::HELP);
    SPIKE_EXPECT(parse({"spike", "-h"}).command == spike::Command::HELP);
}

void test_cli_version_flag() {
    SPIKE_EXPECT(parse({"spike", "--version"}).command == spike::Command::VERSION);
    SPIKE_EXPECT(parse({"spike", "-v"}).command == spike::Command::VERSION);
}

void test_cli_subcommands_no_args() {
    SPIKE_EXPECT(parse({"spike", "fetch"}).command == spike::Command::FETCH);
    SPIKE_EXPECT(parse({"spike", "dev"}).command   == spike::Command::DEV);
    SPIKE_EXPECT(parse({"spike", "random"}).command == spike::Command::RANDOM);
}

void test_cli_tokens_needs_file() {
    auto missing = parse({"spike", "tokens"});
    SPIKE_EXPECT(missing.command == spike::Command::TOKENS);
    SPIKE_EXPECT(missing.parse_error == true);

    auto ok = parse({"spike", "tokens", "x.por"});
    SPIKE_EXPECT(ok.command == spike::Command::TOKENS);
    SPIKE_EXPECT(ok.parse_error == false);
    SPIKE_EXPECT(ok.input_file == "x.por");
    SPIKE_EXPECT(ok.flag_verbose == false);
}

void test_cli_tokens_accepts_verbose() {
    auto a = parse({"spike", "tokens", "x.por", "--verbose"});
    SPIKE_EXPECT(a.command == spike::Command::TOKENS);
    SPIKE_EXPECT(a.parse_error == false);
    SPIKE_EXPECT(a.flag_verbose);
}

void test_cli_tokens_rejects_unknown_flag() {
    auto a = parse({"spike", "tokens", "x.por", "--xpto"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_compile_simple() {
    auto a = parse({"spike", "calc.por"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.input_file == "calc.por");
    SPIKE_EXPECT(a.parse_error == false);
}

void test_cli_compile_case_insensitive_extension() {
    auto a = parse({"spike", "CALC.POR"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.input_file == "CALC.POR");
}

void test_cli_compile_with_all_flags() {
    auto a = parse({"spike", "calc.por", "-o", "saida.exe",
                    "--run", "--time", "--verbose", "--c", "--ast"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.input_file == "calc.por");
    SPIKE_EXPECT(a.output_file == "saida.exe");
    SPIKE_EXPECT(a.flag_run);
    SPIKE_EXPECT(a.flag_time);
    SPIKE_EXPECT(a.flag_verbose);
    SPIKE_EXPECT(a.flag_keep_c);
    SPIKE_EXPECT(a.flag_ast);
    SPIKE_EXPECT(a.parse_error == false);
}

void test_cli_compile_flags_in_any_order() {
    auto a = parse({"spike", "calc.por", "--ast", "-o", "out.exe", "--run"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.output_file == "out.exe");
    SPIKE_EXPECT(a.flag_ast);
    SPIKE_EXPECT(a.flag_run);
}

void test_cli_compile_dash_o_missing_value() {
    auto a = parse({"spike", "calc.por", "-o"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_compile_dash_o_eats_flag_is_error() {
    auto a = parse({"spike", "calc.por", "-o", "--run"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_compile_unknown_flag_is_error() {
    auto a = parse({"spike", "calc.por", "--xpto"});
    SPIKE_EXPECT(a.parse_error == true);
    SPIKE_EXPECT(a.parse_error_msg.find("--xpto") != std::string::npos);
}

void test_cli_random_modes() {
    SPIKE_EXPECT(parse({"spike", "random"}).random_mode == "");
    SPIKE_EXPECT(parse({"spike", "random", "--basico"}).random_mode  == "basico");
    SPIKE_EXPECT(parse({"spike", "random", "--desafio"}).random_mode == "desafio");
    SPIKE_EXPECT(parse({"spike", "random", "--quiz"}).random_mode    == "quiz");
}

void test_cli_random_unknown_mode_is_error() {
    auto a = parse({"spike", "random", "--xpto"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_unknown_command_is_error() {
    auto a = parse({"spike", "foobar"});
    SPIKE_EXPECT(a.command == spike::Command::UNKNOWN);
    SPIKE_EXPECT(a.parse_error == true);
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

    test_lexer_diag_codes_are_wired();
    test_lexer_diagnostics_carries_hints();

    test_diag_empty_starts_clean();
    test_diag_error_counts();
    test_diag_warning_only_does_not_set_has_errors();
    test_diag_handles_out_of_range_line();
    test_diag_print_contains_key_pieces();
    test_diag_no_hint_omits_dica_line();

    test_cli_no_args_is_help();
    test_cli_help_flag();
    test_cli_version_flag();
    test_cli_subcommands_no_args();
    test_cli_tokens_needs_file();
    test_cli_tokens_accepts_verbose();
    test_cli_tokens_rejects_unknown_flag();
    test_cli_compile_simple();
    test_cli_compile_case_insensitive_extension();
    test_cli_compile_with_all_flags();
    test_cli_compile_flags_in_any_order();
    test_cli_compile_dash_o_missing_value();
    test_cli_compile_dash_o_eats_flag_is_error();
    test_cli_compile_unknown_flag_is_error();
    test_cli_random_modes();
    test_cli_random_unknown_mode_is_error();
    test_cli_unknown_command_is_error();

    if (g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << g_failures << " test(s) failed" << std::endl;
    return 1;
}
