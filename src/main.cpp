#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "lexer.hpp"

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitUsageError = 1;
constexpr int kExitFileError = 2;
constexpr int kExitLexerError = 3;

void print_usage(const char* program_name) {
    std::cerr << "Spike 0.1.0\n"
              << "Usage:\n"
              << "  " << program_name << " tokens <file.por>\n"
              << "  " << program_name << " --help\n";
}

std::string read_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("could not open file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

int run_tokens(const std::string& path) {
    const std::string source = read_file(path);
    Lexer lexer(source);

    try {
        const std::vector<Token> tokens = lexer.tokenize();

        for (const Token& token : tokens) {
            const std::string lexeme =
                token.type == TokenType::EndOfFile ? "<eof>" : escape_lexeme(token.lexeme);
            std::ostringstream location;
            location << token.line << ':' << token.column;

            std::cout << std::right << std::setw(8) << location.str() << "  " << std::left
                      << std::setw(22) << token_type_to_string(token.type) << "  \""
                      << lexeme << "\"\n";
        }

        return kExitSuccess;
    } catch (const LexerError& error) {
        std::cerr << path << ':' << error.line() << ':' << error.column() << ": lexer error: "
                  << error.what() << '\n';
        return kExitLexerError;
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        print_usage(argv[0]);
        return kExitSuccess;
    }

    if (argc != 3) {
        print_usage(argv[0]);
        return kExitUsageError;
    }

    const std::string command = argv[1];
    if (command != "tokens") {
        std::cerr << "unknown command: " << command << '\n';
        print_usage(argv[0]);
        return kExitUsageError;
    }

    try {
        return run_tokens(argv[2]);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << '\n';
        return kExitFileError;
    }
}
