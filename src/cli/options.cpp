#include "cli/options.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace portugol::cli {

namespace {

void printCommandHelp(const std::string& topic, std::ostream& out) {
    if (topic == "build") {
        out << "spike build <arquivo.por>\n\nGera codigo C e compila o binario final.\n";
        return;
    }
    if (topic == "run") {
        out << "spike run <arquivo.por>\n\nCompila e executa o programa final.\n";
        return;
    }
    if (topic == "check") {
        out << "spike check <arquivo.por>\n\nExecuta lexer, parser, semantica, passes e IR sem gerar binario.\n";
        return;
    }
    if (topic == "tokens") {
        out << "spike tokens <arquivo.por> [--json] [--with-spans]\n\nMostra os tokens gerados pelo lexer.\n";
        return;
    }
    if (topic == "ast") {
        out << "spike ast <arquivo.por> [--json] [--graph] [--output arquivo]\n\nMostra a AST em texto, JSON ou Graphviz.\n";
        return;
    }
    if (topic == "ir") {
        out << "spike ir <arquivo.por> [--json] [--stdout]\n\nMostra a representacao intermediaria textual do programa.\n";
        return;
    }
    if (topic == "stats") {
        out << "spike stats <arquivo.por> [--json]\n\nMostra estatisticas estruturais do programa.\n";
        return;
    }
    if (topic == "explain") {
        out << "spike explain <arquivo.por>\n\nExplica o pipeline completo: fonte, tokens, AST, IR e C gerado.\n";
        return;
    }
    if (topic == "doctor") {
        out << "spike doctor\n\nVerifica ferramentas do ambiente e disponibilidade do compilador C.\n";
        return;
    }
    if (topic == "version") {
        out << "spike version\n\nMostra a versao atual do Spike.\n";
        return;
    }

    printUsage(out);
}

} // namespace

void printUsage(std::ostream& out) {
    out
        << "Spike - compilador de Portugol para C\n\n"
        << "uso:\n"
        << "  spike <arquivo.por>\n"
        << "  spike build <arquivo.por>\n"
        << "  spike run <arquivo.por>\n"
        << "  spike check <arquivo.por>\n"
        << "  spike tokens <arquivo.por>\n"
        << "  spike ast <arquivo.por>\n"
        << "  spike ir <arquivo.por>\n"
        << "  spike stats <arquivo.por>\n"
        << "  spike explain <arquivo.por>\n"
        << "  spike emit-c <arquivo.por> [--emit-c <arquivo.c>]\n"
        << "  spike trace <arquivo.por>\n\n"
        << "  spike doctor\n"
        << "  spike version\n"
        << "  spike help\n\n"
        << "opcoes:\n"
        << "  --tokens              imprime os tokens e encerra\n"
        << "  --lexer-only          alias para --tokens\n"
        << "  --ast                 imprime a AST e encerra\n"
        << "  --emit-c [arquivo.c]  gera apenas o codigo C\n"
        << "  --no-build            nao chama o compilador C\n"
        << "  --run                 executa o binario gerado\n"
        << "  --json                usa saida em JSON quando suportado\n"
        << "  --graph               gera saida Graphviz quando suportado\n"
        << "  --output <arquivo>    define o caminho de artefato textual\n"
        << "  --stdout              imprime o artefato em stdout\n"
        << "  --with-spans          inclui spans e localizacao quando suportado\n"
        << "  --trace               mostra o inicio e o fim de cada fase\n"
        << "  --time                mostra o tempo gasto em cada fase\n"
        << "  --cc <compilador>     escolhe gcc, clang ou outro compilador C\n"
        << "  -o <binario>          define o caminho do executavel final\n";
}

core::SessionOptions parseOptions(int argc, char** argv) {
    if (argc <= 1) {
        throw std::runtime_error("faltou informar o arquivo de entrada");
    }

    core::SessionOptions options;
    int startIndex = 1;

    const std::string firstArg = argv[1];
    if (firstArg == "--version") {
        options.command = core::SessionCommand::Version;
        startIndex = 2;
    } else if (firstArg == "build") {
        options.command = core::SessionCommand::Build;
        startIndex = 2;
    } else if (firstArg == "run") {
        options.command = core::SessionCommand::Run;
        options.runBinary = true;
        startIndex = 2;
    } else if (firstArg == "check") {
        options.command = core::SessionCommand::Check;
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "tokens") {
        options.command = core::SessionCommand::Tokens;
        options.dumpTokens = true;
        startIndex = 2;
    } else if (firstArg == "ast") {
        options.command = core::SessionCommand::Ast;
        options.dumpAst = true;
        startIndex = 2;
    } else if (firstArg == "ir") {
        options.command = core::SessionCommand::Ir;
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "stats") {
        options.command = core::SessionCommand::Stats;
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "explain") {
        options.command = core::SessionCommand::Explain;
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "emit-c") {
        options.command = core::SessionCommand::EmitC;
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "trace") {
        options.command = core::SessionCommand::Trace;
        options.tracePhases = true;
        startIndex = 2;
    } else if (firstArg == "doctor") {
        options.command = core::SessionCommand::Doctor;
        options.buildBinary = false;
        startIndex = 2;
    } else if (firstArg == "version") {
        options.command = core::SessionCommand::Version;
        startIndex = 2;
    } else if (firstArg == "help") {
        options.command = core::SessionCommand::Help;
        if (argc > 2) {
            options.helpTopic = argv[2];
        }
        if (options.helpTopic.empty()) {
            printUsage(std::cout);
        } else {
            printCommandHelp(options.helpTopic, std::cout);
        }
        std::exit(0);
    }

    for (int index = startIndex; index < argc; ++index) {
        const std::string arg = argv[index];

        if (arg == "-h" || arg == "--help") {
            if (options.inputPath.empty()) {
                printUsage(std::cout);
            } else {
                printCommandHelp(firstArg, std::cout);
            }
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
        if (arg == "--json") {
            options.jsonOutput = true;
            continue;
        }
        if (arg == "--graph") {
            options.graphOutput = true;
            continue;
        }
        if (arg == "--stdout") {
            options.stdoutOutput = true;
            continue;
        }
        if (arg == "--with-spans") {
            options.withSpans = true;
            continue;
        }
        if (arg == "--output") {
            if (index + 1 >= argc) {
                throw std::runtime_error("faltou o caminho apos --output");
            }
            options.artifactOutputPath = argv[++index];
            continue;
        }
        if (arg == "--trace") {
            options.tracePhases = true;
            continue;
        }
        if (arg == "--time") {
            options.showTiming = true;
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

    const bool commandNeedsInput = options.command != core::SessionCommand::Version
        && options.command != core::SessionCommand::Help
        && options.command != core::SessionCommand::Doctor;

    if (commandNeedsInput && options.inputPath.empty()) {
        throw std::runtime_error("faltou informar o arquivo de entrada");
    }

    if (commandNeedsInput && options.cOutputPath.empty()) {
        options.cOutputPath = std::filesystem::path("generated") / (options.inputPath.stem().string() + ".c");
    }

    if (commandNeedsInput && options.command == core::SessionCommand::EmitC && !options.artifactOutputPath.empty()) {
        options.cOutputPath = options.artifactOutputPath;
    }

    if (commandNeedsInput && options.binaryOutputPath.empty()) {
        options.binaryOutputPath = std::filesystem::path("generated") / options.inputPath.stem();
    }

    if (options.runBinary) {
        options.buildBinary = true;
    }

    return options;
}

} // namespace portugol::cli
