#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "spike/file_reader.h"
#include "spike/lexer.h"
#include "spike/token.h"

namespace {

void PrintUsage(const char* program_name) {
    std::cerr << "Uso: " << program_name << " tokens <arquivo.por>\n";
}

void RunTokensCommand(const std::string& path) {
    const std::string source = spike::ReadFile(path);
    spike::Lexer lexer(source);
    const std::vector<spike::Token> tokens = lexer.Tokenize();

    for (const spike::Token& token : tokens) {
        if (token.type == spike::TokenType::EndOfFile) {
            continue;
        }

        std::cout << spike::ToString(token.type) << '\n';
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 3 || std::string(argv[1]) != "tokens") {
        PrintUsage(argv[0]);
        return 1;
    }

    try {
        RunTokensCommand(argv[2]);
    } catch (const std::exception& exception) {
        std::cerr << "Erro: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
