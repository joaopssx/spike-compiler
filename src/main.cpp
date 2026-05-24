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

    print_help();
    return 0;
}
