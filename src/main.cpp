#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "spike/file_reader.h"
#include "spike/lexer.h"
#include "spike/token.h"

namespace {

void PrintUsage(const char* program_name) {
    std::cerr << "Uso: " << program_name << " tokens <arquivo.por> [--verbose]\n";
}

void PrintToken(const spike::Token& token, bool verbose) {
    std::cout << spike::ToString(token.type);
    if (verbose) {
        std::cout << " lexeme=" << std::quoted(token.lexeme) << " line=" << token.line
                  << " column=" << token.column;
    }

    std::cout << '\n';
}

void RunTokensCommand(const std::string& path, bool verbose) {
    const std::string source = spike::ReadFile(path);
    spike::Lexer lexer(source);
    const std::vector<spike::Token> tokens = lexer.Tokenize();

    for (const spike::Token& token : tokens) {
        if (token.type == spike::TokenType::EndOfFile) {
            continue;
        }

        PrintToken(token, verbose);
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 3 || std::string(argv[1]) != "tokens") {
        PrintUsage(argv[0]);
        return 1;
    }

    std::string path;
    bool verbose = false;

    for (int index = 2; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--verbose") {
            verbose = true;
            continue;
        }

        if (path.empty()) {
            path = argument;
            continue;
        }

        PrintUsage(argv[0]);
        return 1;
    }

    if (path.empty()) {
        PrintUsage(argv[0]);
        return 1;
    }

    try {
        RunTokensCommand(path, verbose);
    } catch (const std::exception& exception) {
        std::cerr << "Erro: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
