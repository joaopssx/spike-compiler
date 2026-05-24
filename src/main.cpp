#include "spike/file_reader.hpp"

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
    spike::ReadResult result = spike::read_file(path);
    if (!result.ok) {
        std::cerr << result.error << std::endl;
        return 1;
    }
    if (!result.error.empty()) {
        // Non-fatal warning (e.g. wrong extension).
        std::cerr << result.error << std::endl;
    }
    std::cout << "Lido: " << result.content.size() << " bytes" << std::endl;
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

    // First non-flag argument is treated as the input file.
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            return run_file(arg);
        }
    }

    print_help();
    return 0;
}
