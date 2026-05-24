#include "spike/colors.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <cstring>
#include <iostream>
#include <string>

namespace {

void print_version() {
    std::cout << spike::caramel(std::string("Spike ") + SPIKE_VERSION) << std::endl;
}

void print_help() {
    std::cout
        << spike::caramel(std::string("Spike ") + SPIKE_VERSION)
        << " - compilador de Portugol\n"
        << "\n"
        << spike::bold("Uso:") << "\n"
        << "  spike [opcoes] [arquivo.por]\n"
        << "\n"
        << spike::bold("Opcoes:") << "\n"
        << "  --version    Mostra a versao do compilador\n"
        << "  --help       Mostra esta mensagem de ajuda\n";
}

int run_file(const std::string& path) {
    spike::ReadResult read = spike::read_file(path);
    if (!read.ok) {
        std::cerr << spike::red(read.error) << std::endl;
        return 1;
    }
    if (!read.error.empty()) {
        std::cerr << spike::yellow(read.error) << std::endl;
    }

    spike::Lexer lexer(read.content, path);
    const std::vector<spike::Token> tokens = lexer.tokenize();

    for (const spike::Token& t : tokens) {
        std::cout << spike::gray("[" + std::to_string(t.line) + ":" +
                                 std::to_string(t.col) + "]")
                  << " " << spike::cyan(spike::token_type_to_string(t.type))
                  << " \"" << t.lexeme << "\"\n";
    }

    if (lexer.had_error()) {
        lexer.diagnostics().print_all();
        return 1;
    }
    std::cout << spike::green("Analise lexica concluida sem erros.") << std::endl;
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    spike::init_colors();

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
