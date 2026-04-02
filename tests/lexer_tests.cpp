#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "spike/file_reader.h"
#include "spike/lexer.h"
#include "spike/token.h"

namespace {

const std::string kSourceDir = SPIKE_SOURCE_DIR;

struct TestCase {
    const char* name;
    void (*run)();
};

[[noreturn]] void Fail(const std::string& message) {
    throw std::runtime_error(message);
}

void Assert(bool condition, const std::string& message) {
    if (!condition) {
        Fail(message);
    }
}

bool Contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
}

std::string FixturePath(const std::string& relative_path) {
    return kSourceDir + "/tests/fixtures/" + relative_path;
}

std::vector<spike::Token> TokenizeWithoutEof(const std::string& source) {
    spike::Lexer lexer(source);
    std::vector<spike::Token> tokens = lexer.Tokenize();
    if (!tokens.empty() && tokens.back().type == spike::TokenType::EndOfFile) {
        tokens.pop_back();
    }

    return tokens;
}

std::vector<spike::Token> TokenizeFixtureWithoutEof(const std::string& relative_path) {
    const std::string full_path = FixturePath(relative_path);
    const std::string source = spike::ReadFile(full_path);
    spike::Lexer lexer(source, relative_path);
    std::vector<spike::Token> tokens = lexer.Tokenize();
    if (!tokens.empty() && tokens.back().type == spike::TokenType::EndOfFile) {
        tokens.pop_back();
    }

    return tokens;
}

void ExpectTokenTypes(const std::vector<spike::Token>& tokens,
                      const std::vector<spike::TokenType>& expected_types) {
    if (tokens.size() != expected_types.size()) {
        Fail("unexpected token count");
    }

    for (std::size_t index = 0; index < expected_types.size(); ++index) {
        if (tokens[index].type != expected_types[index]) {
            Fail("unexpected token type at index " + std::to_string(index));
        }
    }
}

void TestKeywordsAndSymbols() {
    const std::string source =
        "algoritmo soma\n"
        "var\n"
        "  x, y: inteiro\n"
        "inicio\n"
        "  leia(x)\n"
        "fimalgoritmo\n";

    const std::vector<spike::Token> tokens = TokenizeWithoutEof(source);

    ExpectTokenTypes(tokens,
                     {
                         spike::TokenType::Algoritmo,
                         spike::TokenType::Identifier,
                         spike::TokenType::Var,
                         spike::TokenType::Identifier,
                         spike::TokenType::Comma,
                         spike::TokenType::Identifier,
                         spike::TokenType::Colon,
                         spike::TokenType::Inteiro,
                         spike::TokenType::Inicio,
                         spike::TokenType::Leia,
                         spike::TokenType::LeftParen,
                         spike::TokenType::Identifier,
                         spike::TokenType::RightParen,
                         spike::TokenType::FimAlgoritmo,
                     });

    Assert(tokens[0].line == 1 && tokens[0].column == 1,
           "expected algoritmo at line 1, column 1");
    Assert(tokens[3].lexeme == "x", "expected first variable name to be x");
    Assert(tokens[7].lexeme == "inteiro", "expected inteiro keyword lexeme");
}

void TestNumbersAndStrings() {
    const std::string source =
        "valor <- 10\n"
        "media <- 3.14\n"
        "escreva(\"oi\")\n";

    const std::vector<spike::Token> tokens = TokenizeWithoutEof(source);

    ExpectTokenTypes(tokens,
                     {
                         spike::TokenType::Identifier,
                         spike::TokenType::Assign,
                         spike::TokenType::Number,
                         spike::TokenType::Identifier,
                         spike::TokenType::Assign,
                         spike::TokenType::Number,
                         spike::TokenType::Escreva,
                         spike::TokenType::LeftParen,
                         spike::TokenType::String,
                         spike::TokenType::RightParen,
                     });

    Assert(tokens[2].lexeme == "10", "expected integer literal lexeme");
    Assert(tokens[2].line == 1 && tokens[2].column == 10,
           "expected integer literal position");
    Assert(tokens[5].lexeme == "3.14", "expected decimal literal lexeme");
    Assert(tokens[8].lexeme == "\"oi\"", "expected string literal lexeme");
    Assert(tokens[8].line == 3 && tokens[8].column == 9,
           "expected string literal position");
}

void TestFixtureCommentsAndEscapes() {
    const std::vector<spike::Token> tokens =
        TokenizeFixtureWithoutEof("valid/comments_and_escapes.por");

    ExpectTokenTypes(tokens,
                     {
                         spike::TokenType::Algoritmo,
                         spike::TokenType::Identifier,
                         spike::TokenType::Var,
                         spike::TokenType::Identifier,
                         spike::TokenType::Colon,
                         spike::TokenType::Texto,
                         spike::TokenType::Inicio,
                         spike::TokenType::Identifier,
                         spike::TokenType::Assign,
                         spike::TokenType::String,
                         spike::TokenType::Escreva,
                         spike::TokenType::LeftParen,
                         spike::TokenType::Identifier,
                         spike::TokenType::RightParen,
                         spike::TokenType::FimAlgoritmo,
                     });

    Assert(tokens[9].type == spike::TokenType::String, "expected string token");
    Assert(tokens[9].lexeme == "\"linha 1\\nlinha 2\\t\\\"ok\\\"\\\\\"",
           "expected escaped string lexeme");
}

void TestFixtureUnterminatedStringError() {
    try {
        const std::vector<spike::Token> tokens =
            TokenizeFixtureWithoutEof("invalid/unterminated_string.por");
        (void)tokens;
        Fail("expected unterminated string error");
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        Assert(Contains(message, "Erro em invalid/unterminated_string.por"),
               "expected file name in unterminated string error");
        Assert(Contains(message, "linha 3, coluna 11"),
               "expected line and column in unterminated string error");
        Assert(Contains(message, "string nao terminada"),
               "expected unterminated string message");
        Assert(Contains(message, "  escreva(\"ola"),
               "expected source line in unterminated string error");
        Assert(Contains(message, "^"), "expected caret indicator in error message");
    }
}

void TestFixtureInvalidEscapeError() {
    try {
        const std::vector<spike::Token> tokens =
            TokenizeFixtureWithoutEof("invalid/invalid_escape.por");
        (void)tokens;
        Fail("expected invalid escape error");
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        Assert(Contains(message, "Erro em invalid/invalid_escape.por"),
               "expected file name in invalid escape error");
        Assert(Contains(message, "sequencia de escape invalida: \\q"),
               "expected invalid escape message");
        Assert(Contains(message, "  escreva(\"oi\\q\")"),
               "expected source line in invalid escape error");
    }
}

void TestFixtureUnexpectedCharacterError() {
    try {
        const std::vector<spike::Token> tokens =
            TokenizeFixtureWithoutEof("invalid/unexpected_character.por");
        (void)tokens;
        Fail("expected unexpected character error");
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        Assert(Contains(message, "Erro em invalid/unexpected_character.por"),
               "expected file name in invalid character error");
        Assert(Contains(message, "linha 3, coluna 3"),
               "expected line and column in invalid character error");
        Assert(Contains(message, "caractere inesperado: '@'"),
               "expected unexpected character message");
        Assert(Contains(message, "  @"), "expected invalid source line in error message");
    }
}

}  // namespace

int main() {
    const std::vector<TestCase> tests = {
        {"keywords and symbols", TestKeywordsAndSymbols},
        {"numbers and strings", TestNumbersAndStrings},
        {"comments and escapes fixture", TestFixtureCommentsAndEscapes},
        {"unterminated string fixture", TestFixtureUnterminatedStringError},
        {"invalid escape fixture", TestFixtureInvalidEscapeError},
        {"unexpected character fixture", TestFixtureUnexpectedCharacterError},
    };

    for (const TestCase& test : tests) {
        try {
            test.run();
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
            return 1;
        }
    }

    std::cout << "All lexer tests passed (" << tests.size() << ").\n";
    return 0;
}
