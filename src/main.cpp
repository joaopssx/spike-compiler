#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "utils.hpp"

namespace portugol {

namespace {

struct CliOptions {
    std::filesystem::path inputPath;
    std::filesystem::path cOutputPath;
    std::filesystem::path binaryOutputPath;
    std::string cCompiler {"cc"};
    bool dumpTokens {false};
    bool dumpAst {false};
    bool buildBinary {true};
    bool runBinary {false};
};

void printUsage() {
    std::cout
        << "Spike - compilador de Portugol para C\n\n"
        << "uso:\n"
        << "  spike <arquivo.por>\n"
        << "  spike build <arquivo.por>\n"
        << "  spike run <arquivo.por>\n"
        << "  spike tokens <arquivo.por>\n"
        << "  spike ast <arquivo.por>\n"
        << "  spike emit-c <arquivo.por> [--emit-c <arquivo.c>]\n\n"
        << "opcoes:\n"
        << "  --tokens              imprime os tokens e encerra\n"
        << "  --lexer-only          alias para --tokens\n"
        << "  --ast                 imprime a AST e encerra\n"
        << "  --emit-c [arquivo.c]  gera apenas o codigo C\n"
        << "  --no-build            nao chama o compilador C\n"
        << "  --run                 executa o binario gerado\n"
        << "  --cc <compilador>     escolhe gcc, clang ou outro compilador C\n"
        << "  -o <binario>          define o caminho do executavel final\n";
}

std::string quotePath(const std::filesystem::path& path) {
    return "\"" + path.string() + "\"";
}

std::string executableCommand(const std::filesystem::path& path) {
    if (path.is_absolute() || path.has_parent_path()) {
        return quotePath(path);
    }

    return "./" + quotePath(path);
}

CliOptions parseCli(int argc, char** argv) {
    if (argc <= 1) {
        throw std::runtime_error("faltou informar o arquivo de entrada");
    }

    CliOptions options;
    int startIndex = 1;

    const std::string firstArg = argv[1];
    if (firstArg == "build") {
        startIndex = 2;
    } else if (firstArg == "run") {
        options.runBinary = true;
        startIndex = 2;
    } else if (firstArg == "tokens") {
        options.dumpTokens = true;
        startIndex = 2;
    } else if (firstArg == "ast") {
        options.dumpAst = true;
        startIndex = 2;
    } else if (firstArg == "emit-c") {
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "help") {
        printUsage();
        std::exit(0);
    }

    for (int index = startIndex; index < argc; ++index) {
        const std::string arg = argv[index];

        if (arg == "-h" || arg == "--help") {
            printUsage();
            std::exit(0);
        }
        if (arg == "--tokens") {
            options.dumpTokens = true;
            continue;
        }
        if (arg == "--lexer-only") {
            options.dumpTokens = true;
            options.buildBinary = false;
            continue;
        }
        if (arg == "--ast") {
            options.dumpAst = true;
            continue;
        }
        if (arg == "--compile") {
            options.buildBinary = true;
            continue;
        }
        if (arg == "--emit-c") {
            options.buildBinary = false;
            if (index + 1 < argc) {
                const std::string nextArg = argv[index + 1];
                if (!nextArg.empty() && nextArg[0] != '-') {
                    options.cOutputPath = argv[++index];
                }
            }
            continue;
        }
        if (arg == "--no-build") {
            options.buildBinary = false;
            continue;
        }
        if (arg == "--run") {
            options.runBinary = true;
            continue;
        }
        if (arg == "--cc") {
            if (index + 1 >= argc) {
                throw std::runtime_error("faltou o nome do compilador apos --cc");
            }
            options.cCompiler = argv[++index];
            continue;
        }
        if (arg == "-o") {
            if (index + 1 >= argc) {
                throw std::runtime_error("faltou o caminho do binario apos -o");
            }
            options.binaryOutputPath = argv[++index];
            continue;
        }
        if (!arg.empty() && arg[0] == '-') {
            throw std::runtime_error("opcao desconhecida: " + arg);
        }
        if (!options.inputPath.empty()) {
            throw std::runtime_error("mais de um arquivo de entrada foi informado");
        }

        options.inputPath = arg;
    }

    if (options.inputPath.empty()) {
        throw std::runtime_error("faltou informar o arquivo de entrada");
    }

    if (options.cOutputPath.empty()) {
        options.cOutputPath = std::filesystem::path("generated") / (options.inputPath.stem().string() + ".c");
    }

    if (options.binaryOutputPath.empty()) {
        options.binaryOutputPath = std::filesystem::path("generated") / options.inputPath.stem();
    }

    if (options.runBinary) {
        options.buildBinary = true;
    }

    return options;
}

} // namespace

} // namespace portugol

int main(int argc, char** argv) {
    using namespace portugol;

    std::string source;
    std::string inputName = "<desconhecido>";

    try {
        const CliOptions options = parseCli(argc, argv);
        inputName = options.inputPath.string();
        source = readFile(options.inputPath);

        Lexer lexer(source);
        const std::vector<Token> tokens = lexer.tokenize();
        if (options.dumpTokens) {
            for (const auto& token : tokens) {
                std::cout << tokenToString(token) << "\n";
            }
            if (!options.dumpAst) {
                return 0;
            }

            std::cout << "\n";
        }

        Parser parser(tokens);
        Program program = parser.parseProgram();
        if (options.dumpAst) {
            std::cout << "AST\n" << dumpAst(program) << "\n";
            return 0;
        }

        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyze(program);

        CodeGenerator codeGenerator;
        const std::string generatedCode = codeGenerator.generate(program);
        writeFile(options.cOutputPath, generatedCode);

        std::cout << "Arquivo C gerado em " << options.cOutputPath.string() << "\n";

        if (options.buildBinary) {
            const std::string command = options.cCompiler + " " + quotePath(options.cOutputPath) + " -o " + quotePath(options.binaryOutputPath);
            std::cout.flush();
            const int result = std::system(command.c_str());
            if (result != 0) {
                throw std::runtime_error("falha ao chamar o compilador C com o comando: " + command);
            }

            std::cout << "Binario gerado em " << options.binaryOutputPath.string() << "\n";

            if (options.runBinary) {
                const std::string runCommand = executableCommand(options.binaryOutputPath);
                std::cout.flush();
                const int runResult = std::system(runCommand.c_str());
                if (runResult != 0) {
                    throw std::runtime_error("falha ao executar o binario gerado com o comando: " + runCommand);
                }
            }
        }

        return 0;
    } catch (const CompilerError& error) {
        std::cerr << formatDiagnostic(inputName, source, error);
    } catch (const std::exception& error) {
        std::cerr << "erro: " << error.what() << "\n";
        std::cerr << "dica: use --help para ver a sintaxe da CLI\n";
    }

    return 1;
}
