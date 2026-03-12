#pragma once

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

#include "core/diagnostics.hpp"
#include "core/phase_profiler.hpp"
#include "core/source_manager.hpp"

namespace portugol::core {

enum class SessionCommand {
    Build,
    Run,
    Check,
    Tokens,
    Ast,
    Ir,
    Stats,
    Explain,
    EmitC,
    Trace,
    Version,
    Help,
    Doctor,
};

struct SessionOptions {
    SessionCommand command {SessionCommand::Build};
    std::filesystem::path inputPath;
    std::filesystem::path cOutputPath;
    std::filesystem::path binaryOutputPath;
    std::string cCompiler {"cc"};
    bool dumpTokens {false};
    bool dumpAst {false};
    bool buildBinary {true};
    bool runBinary {false};
    bool tracePhases {false};
    bool showTiming {false};
    bool jsonOutput {false};
    bool graphOutput {false};
    bool stdoutOutput {false};
    bool withSpans {false};
    std::filesystem::path artifactOutputPath;
    std::string helpTopic;
};

class CompilationSession {
public:
    explicit CompilationSession(SessionOptions options);

    SessionOptions& options() noexcept;
    const SessionOptions& options() const noexcept;

    SourceManager& sourceManager() noexcept;
    const SourceManager& sourceManager() const noexcept;

    DiagnosticEngine& diagnostics() noexcept;
    const DiagnosticEngine& diagnostics() const noexcept;

    PhaseProfiler& profiler() noexcept;
    const PhaseProfiler& profiler() const noexcept;

    void printTimingReport(std::ostream& out) const;

    template <typename Func>
    decltype(auto) runPhase(const std::string& phaseName, Func&& func) {
        if (options_.tracePhases) {
            diagnostics_.trace(phaseName, "started", std::cerr);
        }

        const auto startedAt = std::chrono::high_resolution_clock::now();

        if constexpr (std::is_void_v<std::invoke_result_t<Func>>) {
            try {
                std::forward<Func>(func)();
            } catch (...) {
                const auto endedAt = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endedAt - startedAt);
                profiler_.record(phaseName, duration);
                if (options_.tracePhases) {
                    diagnostics_.trace(phaseName, "failed after " + PhaseProfiler::formatDuration(duration), std::cerr);
                }
                throw;
            }

            const auto endedAt = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endedAt - startedAt);
            profiler_.record(phaseName, duration);

            if (options_.tracePhases) {
                diagnostics_.trace(phaseName, "finished in " + PhaseProfiler::formatDuration(duration), std::cerr);
            }

            return;
        } else {
            try {
                auto result = std::forward<Func>(func)();
                const auto endedAt = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endedAt - startedAt);
                profiler_.record(phaseName, duration);

                if (options_.tracePhases) {
                    diagnostics_.trace(phaseName, "finished in " + PhaseProfiler::formatDuration(duration), std::cerr);
                }

                return result;
            } catch (...) {
                const auto endedAt = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endedAt - startedAt);
                profiler_.record(phaseName, duration);
                if (options_.tracePhases) {
                    diagnostics_.trace(phaseName, "failed after " + PhaseProfiler::formatDuration(duration), std::cerr);
                }
                throw;
            }
        }
    }

private:
    SessionOptions options_;
    SourceManager sourceManager_;
    DiagnosticEngine diagnostics_;
    PhaseProfiler profiler_;
};

} // namespace portugol::core
