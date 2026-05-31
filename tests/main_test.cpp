// Lightweight test runner for the Spike Compiler.
// Each test increments a failure counter on assertion error; the runner ends
// with "all tests passed" iff zero failures occurred, which is what CTest
// matches against.

#include "spike/ast.hpp"
#include "spike/cli.hpp"
#include "spike/diagnostics.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"
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

// ---- ast (PROMPT 013) -----------------------------------------------------

void test_ast_leaf_exprs_carry_position() {
    auto n = spike::make_expr(spike::NumberExpr{42.0, true, 3, 5});
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(n->node));
    const auto& num = std::get<spike::NumberExpr>(n->node);
    SPIKE_EXPECT(num.value == 42.0);
    SPIKE_EXPECT(num.is_integer == true);
    SPIKE_EXPECT(num.line == 3);
    SPIKE_EXPECT(num.col == 5);

    auto s = spike::make_expr(spike::StringExpr{"oi", 1, 1});
    SPIKE_EXPECT(std::get<spike::StringExpr>(s->node).value == "oi");

    auto b = spike::make_expr(spike::BoolExpr{true, 1, 1});
    SPIKE_EXPECT(std::get<spike::BoolExpr>(b->node).value);
}

void test_ast_binary_expr_is_recursive() {
    // Constroi a árvore para "x + 1"
    auto left  = spike::make_expr(spike::VarExpr{"x", 1, 1});
    auto right = spike::make_expr(spike::NumberExpr{1.0, true, 1, 5});
    auto sum   = spike::make_expr(
        spike::BinaryExpr{"+", std::move(left), std::move(right), 1, 3});

    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(sum->node));
    const auto& bin = std::get<spike::BinaryExpr>(sum->node);
    SPIKE_EXPECT(bin.op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(bin.left->node));
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(bin.right->node));
}

// ---- parser (PROMPT 014) --------------------------------------------------

// Helper: lex + parse `src` e retorna o Program junto com o parser pra
// inspecionar diagnósticos.
struct ParseResult {
    spike::Program program;
    bool had_error;
    std::string diag_output;
};

ParseResult parse_source(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    auto tokens = lex.tokenize();
    spike::Parser parser(std::move(tokens), "<test>", src);
    ParseResult r;
    r.program = parser.parse();
    r.had_error = parser.had_error();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    parser.diagnostics().print_all();
    std::cerr.rdbuf(old);
    r.diag_output = sink.str();
    return r;
}

void test_parser_empty_program() {
    auto r = parse_source("algoritmo \"vazio\"\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.name == "vazio");
    SPIKE_EXPECT(r.program.globals.empty());
    SPIKE_EXPECT(r.program.body.empty());
}

void test_parser_single_var_decl() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 1);
    SPIKE_EXPECT(r.program.globals[0].name == "x");
    SPIKE_EXPECT(r.program.globals[0].type == "inteiro");
    SPIKE_EXPECT(r.program.globals[0].initializer == nullptr);
}

void test_parser_multi_name_var_decl() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   a, b, c : real\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 3);
    SPIKE_EXPECT(r.program.globals[0].name == "a");
    SPIKE_EXPECT(r.program.globals[1].name == "b");
    SPIKE_EXPECT(r.program.globals[2].name == "c");
    for (const auto& g : r.program.globals) {
        SPIKE_EXPECT(g.type == "real");
    }
}

void test_parser_multiple_var_lines() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : inteiro\n"
        "   nome : caractere\n"
        "   ok : logico\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 3);
    SPIKE_EXPECT(r.program.globals[0].type == "inteiro");
    SPIKE_EXPECT(r.program.globals[1].type == "caractere");
    SPIKE_EXPECT(r.program.globals[2].type == "logico");
}

void test_parser_E100_missing_algoritmo() {
    auto r = parse_source("inicio fimalgoritmo");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E100") != std::string::npos);
}

void test_parser_E101_missing_name() {
    auto r = parse_source("algoritmo inicio fimalgoritmo");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E101") != std::string::npos);
}

void test_parser_E102_missing_inicio() {
    auto r = parse_source("algoritmo \"t\"\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E102") != std::string::npos);
}

void test_parser_E103_missing_fimalgoritmo() {
    auto r = parse_source("algoritmo \"t\"\ninicio\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E103") != std::string::npos);
}

void test_parser_E104_bad_type() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : numero\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E104") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("numero") != std::string::npos);
}

void test_parser_E105_missing_colon() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E105") != std::string::npos);
}

// ---- parser: statements (PROMPT 016) --------------------------------------

void test_stmt_assignment_arrow() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   x <- 5\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(r.program.body[0]->node));
    const auto& a = std::get<spike::AssignStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(a.var_name == "x");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(a.value->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(a.value->node).value == 5.0);
}

void test_stmt_assignment_pascal_style() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   x := 7\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    const auto& a = std::get<spike::AssignStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(a.var_name == "x");
    SPIKE_EXPECT(std::get<spike::NumberExpr>(a.value->node).value == 7.0);
}

void test_stmt_escreva_two_args() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreva(\"oi\", x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(r.program.body[0]->node));
    const auto& e = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(e.newline == false);
    SPIKE_EXPECT(e.args.size() == 2);
    SPIKE_EXPECT(std::holds_alternative<spike::StringExpr>(e.args[0]->node));
    SPIKE_EXPECT(std::get<spike::StringExpr>(e.args[0]->node).value == "oi");
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(e.args[1]->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(e.args[1]->node).name == "x");
}

void test_stmt_escreval_sets_newline() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreval(\"linha\")\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& e = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(e.newline == true);
    SPIKE_EXPECT(e.args.size() == 1);
}

void test_stmt_escreva_empty_args() {
    // escreval() é útil pra imprimir linha em branco — não deve dar erro.
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreval()\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& e = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(e.args.empty());
    SPIKE_EXPECT(e.newline == true);
}

void test_stmt_leia_two_names() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   leia(x, y)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::LeiaStmt>(r.program.body[0]->node));
    const auto& l = std::get<spike::LeiaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(l.var_names.size() == 2);
    SPIKE_EXPECT(l.var_names[0] == "x");
    SPIKE_EXPECT(l.var_names[1] == "y");
}

void test_stmt_multiple_statements_in_body() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var x : inteiro\n"
        "inicio\n"
        "   x <- 1\n"
        "   escreva(x)\n"
        "   leia(x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 3);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(r.program.body[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(r.program.body[1]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::LeiaStmt>(r.program.body[2]->node));
}

void test_stmt_E300_unknown_statement() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   42\n"           // 42 sozinho não é statement válido
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E300") != std::string::npos);
}

void test_stmt_E301_escreva_missing_lparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreva \"oi\")\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E301") != std::string::npos);
}

void test_stmt_E302_escreva_missing_rparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreva(\"oi\"\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E302") != std::string::npos);
}

void test_stmt_E303_leia_missing_lparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   leia x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E303") != std::string::npos);
}

void test_stmt_E304_leia_missing_rparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   leia(x\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E304") != std::string::npos);
}

// ---- parser: expressions (PROMPT 015) -------------------------------------

// Helper: lex `src` e parseia uma única expressão.
struct ExprResult {
    spike::ExprPtr expr;
    bool had_error;
};

ExprResult parse_expression(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    auto tokens = lex.tokenize();
    spike::Parser parser(std::move(tokens), "<test>", src);
    ExprResult r;
    r.expr = parser.parse_expr();
    r.had_error = parser.had_error();
    return r;
}

void test_expr_number_literal() {
    auto r = parse_expression("42");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.expr != nullptr);
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(r.expr->node));
    const auto& n = std::get<spike::NumberExpr>(r.expr->node);
    SPIKE_EXPECT(n.value == 42.0);
    SPIKE_EXPECT(n.is_integer == true);
}

void test_expr_real_literal() {
    auto r = parse_expression("3.14");
    SPIKE_EXPECT(r.had_error == false);
    const auto& n = std::get<spike::NumberExpr>(r.expr->node);
    SPIKE_EXPECT(n.value > 3.13 && n.value < 3.15);
    SPIKE_EXPECT(n.is_integer == false);
}

void test_expr_var() {
    auto r = parse_expression("contador");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(r.expr->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(r.expr->node).name == "contador");
}

void test_expr_bool_literals() {
    {
        auto r = parse_expression("verdadeiro");
        SPIKE_EXPECT(r.had_error == false);
        SPIKE_EXPECT(std::get<spike::BoolExpr>(r.expr->node).value == true);
    }
    {
        auto r = parse_expression("falso");
        SPIKE_EXPECT(r.had_error == false);
        SPIKE_EXPECT(std::get<spike::BoolExpr>(r.expr->node).value == false);
    }
}

void test_expr_addition() {
    auto r = parse_expression("x + 1");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
    const auto& b = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(b.op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(b.left->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(b.left->node).name == "x");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(b.right->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(b.right->node).value == 1.0);
}

void test_expr_unary_not() {
    auto r = parse_expression("nao verdadeiro");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::UnaryExpr>(r.expr->node));
    const auto& u = std::get<spike::UnaryExpr>(r.expr->node);
    SPIKE_EXPECT(u.op == "nao");
    SPIKE_EXPECT(std::holds_alternative<spike::BoolExpr>(u.operand->node));
    SPIKE_EXPECT(std::get<spike::BoolExpr>(u.operand->node).value == true);
}

void test_expr_paren_changes_precedence() {
    // (3 + 4) * 2 → raiz é o '*', filho esquerdo é o '+'.
    auto r = parse_expression("(3 + 4) * 2");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
    const auto& mul = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(mul.op == "*");
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(mul.left->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(mul.left->node).op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(mul.right->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(mul.right->node).value == 2.0);
}

void test_expr_precedence_without_parens() {
    // 1 + 2 * 3 → 1 + (2 * 3); raiz é '+'.
    auto r = parse_expression("1 + 2 * 3");
    SPIKE_EXPECT(r.had_error == false);
    const auto& add = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(add.op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(add.left->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(add.left->node).value == 1.0);
    const auto& mul = std::get<spike::BinaryExpr>(add.right->node);
    SPIKE_EXPECT(mul.op == "*");
}

void test_expr_comparison_lower_than_arithmetic() {
    // a + 1 < b * 2 → comparação na raiz.
    auto r = parse_expression("a + 1 < b * 2");
    SPIKE_EXPECT(r.had_error == false);
    const auto& cmp = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(cmp.op == "<");
}

void test_expr_and_lower_than_comparison() {
    // a > 1 e b < 2 → 'e' na raiz, comparações nos filhos.
    auto r = parse_expression("a > 1 e b < 2");
    SPIKE_EXPECT(r.had_error == false);
    const auto& and_node = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(and_node.op == "e");
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(and_node.left->node).op == ">");
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(and_node.right->node).op == "<");
}

void test_expr_or_keyword_case_insensitive() {
    // Lexer normaliza keywords no match; canonical_op força lowercase no op.
    auto r = parse_expression("verdadeiro OU falso");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(r.expr->node).op == "ou");
}

void test_expr_unary_minus() {
    auto r = parse_expression("-x");
    SPIKE_EXPECT(r.had_error == false);
    const auto& u = std::get<spike::UnaryExpr>(r.expr->node);
    SPIKE_EXPECT(u.op == "-");
    SPIKE_EXPECT(std::get<spike::VarExpr>(u.operand->node).name == "x");
}

void test_expr_unclosed_paren_emits_E200() {
    spike::Lexer lex("(1 + 2", "<test>");
    auto toks = lex.tokenize();
    spike::Parser p(std::move(toks), "<test>", "(1 + 2");
    p.parse_expr();
    SPIKE_EXPECT(p.had_error());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    p.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("E200") != std::string::npos);
}

void test_expr_unexpected_token_emits_E201() {
    spike::Lexer lex("+", "<test>");
    auto toks = lex.tokenize();
    spike::Parser p(std::move(toks), "<test>", "+");
    p.parse_expr();
    SPIKE_EXPECT(p.had_error());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    p.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("E201") != std::string::npos);
}

void test_parser_recovers_to_next_line_after_var_error() {
    // Linha ruim, depois uma linha boa — a boa deve ser parseada.
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x foobar lixo\n"            // erro: sem ':'
        "   y : inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    // y deveria ter sido capturado mesmo depois do erro em x.
    bool found_y = false;
    for (const auto& g : r.program.globals) {
        if (g.name == "y" && g.type == "inteiro") found_y = true;
    }
    SPIKE_EXPECT(found_y);
}

void test_ast_program_assembles() {
    spike::Program prog;
    prog.name = "calc";
    prog.globals.push_back(spike::VarDecl{"x", "inteiro", nullptr, 2, 4});

    // x <- 42
    auto val = spike::make_expr(spike::NumberExpr{42.0, true, 4, 9});
    prog.body.push_back(spike::make_stmt(
        spike::AssignStmt{"x", std::move(val), 4, 4}));

    // se x > 0 entao escreval("positivo") fimse
    auto cond = spike::make_expr(spike::BinaryExpr{
        ">",
        spike::make_expr(spike::VarExpr{"x", 5, 7}),
        spike::make_expr(spike::NumberExpr{0.0, true, 5, 11}),
        5, 9});

    spike::StmtList then_block;
    spike::EscrevaStmt esc;
    esc.newline = true;
    esc.line = 6; esc.col = 7;
    esc.args.push_back(spike::make_expr(spike::StringExpr{"positivo", 6, 16}));
    then_block.push_back(spike::make_stmt(std::move(esc)));

    prog.body.push_back(spike::make_stmt(spike::SeStmt{
        std::move(cond), std::move(then_block), {}, 5, 4}));

    // Verifica que ficou tudo no lugar.
    SPIKE_EXPECT(prog.name == "calc");
    SPIKE_EXPECT(prog.globals.size() == 1);
    SPIKE_EXPECT(prog.globals[0].name == "x");
    SPIKE_EXPECT(prog.body.size() == 2);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(prog.body[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::SeStmt>(prog.body[1]->node));

    const auto& se = std::get<spike::SeStmt>(prog.body[1]->node);
    SPIKE_EXPECT(se.then_branch.size() == 1);
    SPIKE_EXPECT(se.else_branch.empty());
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

    test_ast_leaf_exprs_carry_position();
    test_ast_binary_expr_is_recursive();
    test_ast_program_assembles();

    test_parser_empty_program();
    test_parser_single_var_decl();
    test_parser_multi_name_var_decl();
    test_parser_multiple_var_lines();
    test_parser_E100_missing_algoritmo();
    test_parser_E101_missing_name();
    test_parser_E102_missing_inicio();
    test_parser_E103_missing_fimalgoritmo();
    test_parser_E104_bad_type();
    test_parser_E105_missing_colon();
    test_parser_recovers_to_next_line_after_var_error();

    test_expr_number_literal();
    test_expr_real_literal();
    test_expr_var();
    test_expr_bool_literals();
    test_expr_addition();
    test_expr_unary_not();
    test_expr_paren_changes_precedence();
    test_expr_precedence_without_parens();
    test_expr_comparison_lower_than_arithmetic();
    test_expr_and_lower_than_comparison();
    test_expr_or_keyword_case_insensitive();
    test_expr_unary_minus();
    test_expr_unclosed_paren_emits_E200();
    test_expr_unexpected_token_emits_E201();

    test_stmt_assignment_arrow();
    test_stmt_assignment_pascal_style();
    test_stmt_escreva_two_args();
    test_stmt_escreval_sets_newline();
    test_stmt_escreva_empty_args();
    test_stmt_leia_two_names();
    test_stmt_multiple_statements_in_body();
    test_stmt_E300_unknown_statement();
    test_stmt_E301_escreva_missing_lparen();
    test_stmt_E302_escreva_missing_rparen();
    test_stmt_E303_leia_missing_lparen();
    test_stmt_E304_leia_missing_rparen();

    if (g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << g_failures << " test(s) failed" << std::endl;
    return 1;
}
