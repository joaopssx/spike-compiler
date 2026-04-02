#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "spike/file_reader.h"
#include "spike/lexer.h"
#include "spike/token.h"

namespace {

constexpr const char* kVersion = "0.1.3";

void PrintUsage(std::ostream& output, const char* program_name) {
    output << "Uso:\n"
           << "  " << program_name << " tokens <arquivo.por> [--verbose]\n"
           << "  " << program_name << " --help\n"
           << "  " << program_name << " --version\n";
}

void PrintHelp(const char* program_name) {
    PrintUsage(std::cout, program_name);
    std::cout << "\nComandos:\n"
              << "  tokens     Le um arquivo .por e imprime os tokens\n"
              << "\nOpcoes:\n"
              << "  --verbose  Mostra lexema, linha e coluna para cada token\n";
}

void PrintVersion() { std::cout << "Spike " << kVersion << '\n'; }

void PrintToken(const spike::Token& token, bool verbose) {
    std::cout << spike::ToString(token.type);
    if (verbose) {
        std::cout << " lexeme=" << std::quoted(token.lexeme) << " line=" << token.line
                  << " column=" << token.column;
    }

    std::cout << '\n';
}

int ParseTokensCommand(int argc, char* argv[]) {
    std::string path;
    bool verbose = false;

    for (int index = 2; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--help") {
            PrintUsage(std::cout, argv[0]);
            return 0;
        }

        if (argument == "--verbose") {
            verbose = true;
            continue;
        }

        if (path.empty()) {
            path = argument;
            continue;
        }

        PrintUsage(std::cerr, argv[0]);
        return 1;
    }

    if (path.empty()) {
        PrintUsage(std::cerr, argv[0]);
        return 1;
    }

    const std::string source = spike::ReadFile(path);
    spike::Lexer lexer(source, path);
    const std::vector<spike::Token> tokens = lexer.Tokenize();

    for (const spike::Token& token : tokens) {
        if (token.type == spike::TokenType::EndOfFile) {
            continue;
        }

        PrintToken(token, verbose);
    }

    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(std::cerr, argv[0]);
        return 1;
    }

    const std::string command = argv[1];
    if (command == "--help") {
        PrintHelp(argv[0]);
        return 0;
    }

    if (command == "--version") {
        PrintVersion();
        return 0;
    }

    if (command != "tokens") {
        PrintUsage(std::cerr, argv[0]);
        return 1;
    }

    try {
        return ParseTokensCommand(argc, argv);
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
