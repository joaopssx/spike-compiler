#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <cstring>
#include <iostream>
#include <string>

namespace {

void print_version() {
    std::cout << "Spike " << SPIKE_VERSION << std::endl;
}

void print_help() {
    std::cout
        << "Spike " << SPIKE_VERSION << " - compilador de Portugol\n"
        << "\n"
        << "Uso:\n"
        << "  spike [opcoes] [arquivo.por]\n"
        << "\n"
        << "Opcoes:\n"
        << "  --version    Mostra a versao do compilador\n"
        << "  --help       Mostra esta mensagem de ajuda\n";
}

int run_file(const std::string& path) {
    spike::ReadResult read = spike::read_file(path);
    if (!read.ok) {
        std::cerr << read.error << std::endl;
        return 1;
    }
    if (!read.error.empty()) {
        std::cerr << read.error << std::endl;
    }

    spike::Lexer lexer(read.content, path);
    const std::vector<spike::Token> tokens = lexer.tokenize();

    for (const spike::Token& t : tokens) {
        std::cout << "[" << t.line << ":" << t.col << "] "
                  << spike::token_type_to_string(t.type)
                  << " \"" << t.lexeme << "\"\n";
    }

    if (lexer.had_error()) {
        for (const std::string& err : lexer.errors()) {
            std::cerr << err << std::endl;
        }
        return 1;
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "--version") == 0) {
            print_version();
            return 0;
        }
        if (std::strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        }
    }

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            return run_file(arg);
        }
    }

    print_help();
    return 0;
}
