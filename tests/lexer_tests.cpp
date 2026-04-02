#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "spike/lexer.h"
#include "spike/token.h"

namespace {

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

std::vector<spike::Token> TokenizeWithoutEof(const std::string& source) {
    spike::Lexer lexer(source);
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

void TestLineCommentsAreIgnored() {
    const std::string source =
        "algoritmo teste // nome do programa\n"
        "var // secao de variaveis\n"
        "  x: inteiro // declaracao\n"
        "inicio\n"
        "  // leitura do valor\n"
        "  leia(x) // fim da linha\n"
        "fimalgoritmo\n";

    const std::vector<spike::Token> tokens = TokenizeWithoutEof(source);

    ExpectTokenTypes(tokens,
                     {
                         spike::TokenType::Algoritmo,
                         spike::TokenType::Identifier,
                         spike::TokenType::Var,
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
}

void TestUnterminatedStringError() {
    try {
        const std::vector<spike::Token> tokens = TokenizeWithoutEof("escreva(\"ola");
        (void)tokens;
        Fail("expected unterminated string error");
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        Assert(Contains(message, "linha 1, coluna 9"),
               "expected line and column in unterminated string error");
        Assert(Contains(message, "string nao terminada"),
               "expected unterminated string message");
        Assert(Contains(message, "escreva(\"ola"),
               "expected source line in unterminated string error");
        Assert(Contains(message, "^"), "expected caret indicator in error message");
    }
}

void TestUnexpectedCharacterError() {
    try {
        const std::vector<spike::Token> tokens = TokenizeWithoutEof("@");
        (void)tokens;
        Fail("expected unexpected character error");
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        Assert(Contains(message, "linha 1, coluna 1"),
               "expected line and column in invalid character error");
        Assert(Contains(message, "caractere inesperado: '@'"),
               "expected unexpected character message");
        Assert(Contains(message, "@"), "expected invalid source line in error message");
    }
}

}  // namespace

int main() {
    const std::vector<TestCase> tests = {
        {"keywords and symbols", TestKeywordsAndSymbols},
        {"numbers and strings", TestNumbersAndStrings},
        {"line comments", TestLineCommentsAreIgnored},
        {"unterminated string error", TestUnterminatedStringError},
        {"unexpected character error", TestUnexpectedCharacterError},
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
