#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/lexer.hpp"

#include <sstream>
#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;

std::string program(const std::string& body, const std::string& procedures = "") {
    return "algoritmo \"t\"\nvar\n  x, contador : inteiro\n  b : logico\n" + procedures + "inicio\n" +
           body + "fimalgoritmo\n";
}

std::size_t count(const std::string& text, const std::string& piece) {
    std::size_t n = 0;
    for (std::size_t at = text.find(piece); at != std::string::npos; at = text.find(piece, at + 1)) ++n;
    return n;
}

bool has(const std::string& text, const std::string& piece) {
    return text.find(piece) != std::string::npos;
}

void test_double_equals_is_e203_without_cascade() {
    auto r = parse_source(program("  se x == 1 entao\n    escreval(1)\n  fimse\n"));
    SPIKE_EXPECT(has(r.diag_output, "[E203]") && count(r.diag_output, "erro[") == 1);
    SPIKE_EXPECT(has(r.diag_output, "usa um único '='"));
    SPIKE_EXPECT(r.program.body.size() == 1 &&
                 std::holds_alternative<spike::SeStmt>(r.program.body[0]->node));
}

void test_statement_hints() {
    const std::string assign = parse_source(program("  x = 5\n")).diag_output;
    SPIKE_EXPECT(has(assign, "[E300]") && has(assign, "para atribuir use '<-' (ou ':='): x <- valor."));
    SPIKE_EXPECT(has(parse_source(program("  x += 1\n")).diag_output, "escreva x <- x + 1."));
    SPIKE_EXPECT(has(parse_source(program("  x++\n")).diag_output, "escreva x <- x + 1."));
    SPIKE_EXPECT(has(parse_source(program("  x--\n")).diag_output, "escreva x <- x - 1."));
    SPIKE_EXPECT(has(parse_source(program("  print\n")).diag_output, "escreva() ou escreval()"));
    SPIKE_EXPECT(has(parse_source(program("  while\n")).diag_output, "enquanto ... faca"));
    SPIKE_EXPECT(has(parse_source(program("  escrva\n")).diag_output, "você quis dizer 'escreva'?"));
    SPIKE_EXPECT(has(parse_source(program("  mostar\n", "procedimento mostra()\ninicio\nfimprocedimento\n"))
                         .diag_output,
                     "você quis dizer 'mostra'?"));
    const std::string far = parse_source(program("  zebra\n")).diag_output;
    SPIKE_EXPECT(has(far, "[E300]") && !has(far, "quis dizer"));
}

void test_misspelled_closer_counts_as_closer() {
    auto r = parse_source(program("  se x > 1 entao\n    escreval(1)\n  fimsi\n  escreval(2)\n"));
    SPIKE_EXPECT(count(r.diag_output, "erro[") == 1);
    SPIKE_EXPECT(has(r.diag_output, "instrução não reconhecida: 'fimsi'"));
    SPIKE_EXPECT(has(r.diag_output, "você quis dizer 'fimse'?"));
    SPIKE_EXPECT(r.program.body.size() == 2);
    auto otherwise = parse_source(program("  se x > 1 entao\n    escreval(1)\n  senoa\n    escreval(2)\n"
                                          "  fimse\n"));
    SPIKE_EXPECT(count(otherwise.diag_output, "erro[") == 1);
    if (otherwise.program.body.size() == 1) {
        const auto& se = std::get<spike::SeStmt>(otherwise.program.body[0]->node);
        SPIKE_EXPECT(se.else_branch.size() == 1);
    }
    auto loop = parse_source(program("  enquanto b faca\n    x <- 1\n  fimenqanto\n"));
    SPIKE_EXPECT(count(loop.diag_output, "erro[") == 1 &&
                 has(loop.diag_output, "você quis dizer 'fimenquanto'?"));
}

void test_type_suggestions() {
    const std::string typo =
        parse_source("algoritmo \"t\"\nvar\n  x : intero\ninicio\nfimalgoritmo\n").diag_output;
    SPIKE_EXPECT(has(typo, "[E104]") && has(typo, "você quis dizer 'inteiro'?"));
    const std::string other =
        parse_source("algoritmo \"t\"\nvar\n  x : float\ninicio\nfimalgoritmo\n").diag_output;
    SPIKE_EXPECT(has(other, "o tipo de número com vírgula é 'real'"));
}

std::string lex_errors(const std::string& source, std::vector<spike::Token>& tokens) {
    spike::Lexer lexer(source, "<test>");
    tokens = lexer.tokenize();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    lexer.diagnostics().print_all();
    std::cerr.rdbuf(old);
    return sink.str();
}

void test_symbols_from_other_languages() {
    std::vector<spike::Token> tokens;
    std::string out = lex_errors("x != 1", tokens);
    SPIKE_EXPECT(has(out, "[E001]") && has(out, "use '<>'"));
    SPIKE_EXPECT(tokens.size() == 4 && tokens[1].type == spike::TokenType::NOT_EQUAL);
    out = lex_errors("a && b || !c", tokens);
    SPIKE_EXPECT(count(out, "erro[E001]") == 3 && has(out, "a palavra 'e'") &&
                 has(out, "a palavra 'ou'") && has(out, "a palavra 'nao'"));
    SPIKE_EXPECT(tokens.size() == 7 && tokens[1].type == spike::TokenType::E &&
                 tokens[3].type == spike::TokenType::OU && tokens[4].type == spike::TokenType::NAO);
    out = lex_errors("x <- 1;", tokens);
    SPIKE_EXPECT(has(out, "Portugol não usa ';'") && tokens.size() == 4);
    out = lex_errors("x @ 1", tokens);
    SPIKE_EXPECT(has(out, "Portugol não usa este caractere"));
}

} // namespace

namespace spike_tests {

void run_parser_suggest_tests() {
    test_double_equals_is_e203_without_cascade();
    test_statement_hints();
    test_misspelled_closer_counts_as_closer();
    test_type_suggestions();
    test_symbols_from_other_languages();
}

} // namespace spike_tests
