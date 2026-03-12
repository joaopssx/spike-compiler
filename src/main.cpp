#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ast_graphviz.hpp"
#include "ast_json.hpp"
#include "cli/options.hpp"
#include "codegen.hpp"
#include "core/environment.hpp"
#include "core/session.hpp"
#include "core/version.hpp"
#include "ir.hpp"
#include "ir_builder.hpp"
#include "lexer.hpp"
#include "pass_manager.hpp"
#include "parser.hpp"
#include "program_stats.hpp"
#include "semantic.hpp"
#include "token_json.hpp"
#include "utils.hpp"

namespace {

std::string quotePath(const std::filesystem::path& path) {
    return "\"" + path.string() + "\"";
}

std::string executableCommand(const std::filesystem::path& path) {
    if (path.is_absolute() || path.has_parent_path()) {
        return quotePath(path);
    }

    return "./" + quotePath(path);
}

void printTimingIfEnabled(const portugol::core::CompilationSession& session, std::ostream& out) {
    if (!session.options().showTiming) {
        return;
    }

    out << "\n";
    session.printTimingReport(out);
}

std::size_t countLines(std::string_view source) {
    if (source.empty()) {
        return 0;
    }

    std::size_t lines = 1;
    for (const char ch : source) {
        if (ch == '\n') {
            ++lines;
        }
    }
    return lines;
}

void emitArtifact(const std::string& content, const portugol::core::SessionOptions& options, const std::filesystem::path& defaultPath, const std::string& label) {
    const std::filesystem::path outputPath = options.artifactOutputPath.empty() ? defaultPath : options.artifactOutputPath;

    if (options.stdoutOutput || outputPath.empty()) {
        std::cout << content;
        if (!content.empty() && content.back() != '\n') {
            std::cout << "\n";
        }
        return;
    }

    portugol::writeFile(outputPath, content);
    std::cout << label << " gerado em " << outputPath.string() << "\n";
}

std::string buildExplainReport(const portugol::core::CompilationSession& session, const std::vector<portugol::Token>& tokens, const portugol::Program& program, const portugol::IRProgram& ir, const portugol::PassReport& passReport, const std::string& generatedCode) {
    const portugol::ProgramStats stats = portugol::collectProgramStats(program, countLines(session.sourceManager().source()), tokens.size());

    std::ostringstream out;
    out << "Spike Explain\n\n";
    out << "Input: " << session.sourceManager().displayPath() << "\n\n";
    out << "Pipeline\n";
    out << "1. Source loaded: " << stats.lineCount << " lines\n";
    out << "2. Lexer produced: " << stats.tokenCount << " tokens\n";
    out << "3. Parser built AST with " << stats.declarationCount << " declarations and " << (stats.statementCount - stats.declarationCount) << " executable statements\n";
    out << "4. Semantic analysis: success\n";
    out << "5. Optimization passes: " << passReport.constantFolds << " constant folds\n";
    out << "6. IR generation: " << ir.instructions.size() << " instructions\n";
    out << "7. Code generation: " << generatedCode.size() << " bytes of C emitted\n\n";
    out << renderProgramStats(stats) << "\n";
    out << "AST Preview\n" << dumpAst(program) << "\n";
    out << "IR Preview\n" << dumpIr(ir) << "\n";
    out << "Generated C\n" << generatedCode;
    if (!generatedCode.empty() && generatedCode.back() != '\n') {
        out << "\n";
    }
    return out.str();
}

} // namespace

int main(int argc, char** argv) {
    using namespace portugol;

    std::unique_ptr<core::CompilationSession> session;

    try {
        session = std::make_unique<core::CompilationSession>(cli::parseOptions(argc, argv));

        if (session->options().command == core::SessionCommand::Version) {
            std::cout << "Spike " << core::versionString() << "\n";
            std::cout << core::tagline() << "\n";
            return 0;
        }

        if (session->options().command == core::SessionCommand::Doctor) {
            std::cout << "Spike Doctor\n\n";
            for (const auto& check : core::Environment::doctorChecks(session->options().cCompiler)) {
                std::cout << check.label << ": " << (check.ok ? "OK" : "FAIL") << " - " << check.detail << "\n";
            }
            return 0;
        }

        session->runPhase("file-read", [&] {
            session->sourceManager().load(session->options().inputPath);
        });

        const std::vector<Token> tokens = session->runPhase("lexer", [&] {
            Lexer lexer(std::string(session->sourceManager().source()));
            return lexer.tokenize();
        });

        if (session->options().dumpTokens) {
            std::ostringstream tokenOut;
            if (session->options().jsonOutput) {
                tokenOut << tokensToJson(tokens);
            } else {
                for (const auto& token : tokens) {
                    tokenOut << tokenToString(token);
                    if (session->options().withSpans) {
                        tokenOut << " [" << token.location.line << ":" << token.location.column << "]";
                    }
                    tokenOut << "\n";
                }
            }
            if (!session->options().dumpAst) {
                if (session->options().artifactOutputPath.empty() && !session->options().stdoutOutput) {
                    std::cout << tokenOut.str();
                } else {
                    emitArtifact(tokenOut.str(), session->options(), std::filesystem::path(), "Tokens");
                }
                printTimingIfEnabled(*session, std::cout);
                return 0;
            }

            std::cout << tokenOut.str() << "\n";
        }

        Program program = session->runPhase("parser", [&] {
            Parser parser(tokens);
            return parser.parseProgram();
        });

        if (session->options().dumpAst) {
            if (session->options().jsonOutput) {
                const std::filesystem::path defaultPath = std::filesystem::path("generated") / (session->options().inputPath.stem().string() + ".ast.json");
                emitArtifact(astToJson(program), session->options(), defaultPath, "AST JSON");
            } else if (session->options().graphOutput) {
                const std::filesystem::path defaultPath = std::filesystem::path("generated") / (session->options().inputPath.stem().string() + ".ast.dot");
                emitArtifact(astToGraphviz(program), session->options(), defaultPath, "AST Graphviz");
            } else {
                std::cout << "AST\n" << dumpAst(program) << "\n";
            }
            printTimingIfEnabled(*session, std::cout);
            return 0;
        }

        session->runPhase("semantic", [&] {
            SemanticAnalyzer semanticAnalyzer;
            semanticAnalyzer.analyze(program);
        });

        const PassReport passReport = session->runPhase("passes", [&] {
            return runOptimizationPasses(program);
        });

        const IRProgram irProgram = session->runPhase("ir", [&] {
            IRBuilder builder;
            return builder.build(program);
        });

        if (session->options().command == core::SessionCommand::Check) {
            std::cout << "Analise concluida com sucesso para " << session->sourceManager().displayPath() << "\n";
            if (passReport.constantFolds > 0) {
                std::cout << "Constant folds aplicados: " << passReport.constantFolds << "\n";
            }
            printTimingIfEnabled(*session, std::cout);
            return 0;
        }

        if (session->options().command == core::SessionCommand::Ir) {
            if (session->options().jsonOutput) {
                const std::filesystem::path defaultPath = std::filesystem::path("generated") / (session->options().inputPath.stem().string() + ".ir.json");
                emitArtifact(irToJson(irProgram), session->options(), defaultPath, "IR JSON");
            } else {
                const std::filesystem::path defaultPath = session->options().stdoutOutput ? std::filesystem::path() : (std::filesystem::path("generated") / (session->options().inputPath.stem().string() + ".ir.txt"));
                emitArtifact(dumpIr(irProgram), session->options(), defaultPath, "IR");
            }
            printTimingIfEnabled(*session, std::cout);
            return 0;
        }

        if (session->options().command == core::SessionCommand::Stats) {
            const ProgramStats stats = collectProgramStats(program, countLines(session->sourceManager().source()), tokens.size());
            if (session->options().jsonOutput) {
                emitArtifact(renderProgramStatsJson(stats), session->options(), std::filesystem::path(), "Stats");
            } else {
                emitArtifact(renderProgramStats(stats), session->options(), std::filesystem::path(), "Stats");
            }
            printTimingIfEnabled(*session, std::cout);
            return 0;
        }

        const std::string generatedCode = session->runPhase("codegen", [&] {
            CodeGenerator codeGenerator;
            return codeGenerator.generate(program);
        });

        if (session->options().command == core::SessionCommand::Explain) {
            emitArtifact(buildExplainReport(*session, tokens, program, irProgram, passReport, generatedCode), session->options(), std::filesystem::path(), "Explain");
            printTimingIfEnabled(*session, std::cout);
            return 0;
        }

        if (session->options().command == core::SessionCommand::EmitC && session->options().stdoutOutput) {
            std::cout << generatedCode;
            if (!generatedCode.empty() && generatedCode.back() != '\n') {
                std::cout << "\n";
            }
            printTimingIfEnabled(*session, std::cout);
            return 0;
        }

        session->runPhase("write-c", [&] {
            writeFile(session->options().cOutputPath, generatedCode);
        });

        std::cout << "Arquivo C gerado em " << session->options().cOutputPath.string() << "\n";

        if (session->options().buildBinary) {
            session->runPhase("c-compile", [&] {
                const std::string command = session->options().cCompiler + " " + quotePath(session->options().cOutputPath) + " -o " + quotePath(session->options().binaryOutputPath);
                std::cout.flush();
                const int result = std::system(command.c_str());
                if (result != 0) {
                    throw std::runtime_error("falha ao chamar o compilador C com o comando: " + command);
                }
            });

            std::cout << "Binario gerado em " << session->options().binaryOutputPath.string() << "\n";

            if (session->options().runBinary) {
                session->runPhase("run", [&] {
                    const std::string runCommand = executableCommand(session->options().binaryOutputPath);
                    std::cout.flush();
                    const int runResult = std::system(runCommand.c_str());
                    if (runResult != 0) {
                        throw std::runtime_error("falha ao executar o binario gerado com o comando: " + runCommand);
                    }
                });
            }
        }

        printTimingIfEnabled(*session, std::cout);
        return 0;
    } catch (const CompilerError& error) {
        if (session != nullptr) {
            session->diagnostics().emitCompilerError(error, std::cerr);
            printTimingIfEnabled(*session, std::cerr);
        } else {
            std::cerr << "error[" << error.stage() << "]: " << error.message() << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "erro: " << error.what() << "\n";
        std::cerr << "dica: use --help para ver a sintaxe da CLI\n";
        if (session != nullptr) {
            printTimingIfEnabled(*session, std::cerr);
        }
    }

    return 1;
}
