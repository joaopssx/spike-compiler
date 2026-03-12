#include "core/session.hpp"

namespace portugol::core {

CompilationSession::CompilationSession(SessionOptions options)
    : options_(std::move(options))
    , diagnostics_(&sourceManager_) {
    diagnostics_.setTraceEnabled(options_.tracePhases);
}

SessionOptions& CompilationSession::options() noexcept {
    return options_;
}

const SessionOptions& CompilationSession::options() const noexcept {
    return options_;
}

SourceManager& CompilationSession::sourceManager() noexcept {
    return sourceManager_;
}

const SourceManager& CompilationSession::sourceManager() const noexcept {
    return sourceManager_;
}

DiagnosticEngine& CompilationSession::diagnostics() noexcept {
    return diagnostics_;
}

const DiagnosticEngine& CompilationSession::diagnostics() const noexcept {
    return diagnostics_;
}

PhaseProfiler& CompilationSession::profiler() noexcept {
    return profiler_;
}

const PhaseProfiler& CompilationSession::profiler() const noexcept {
    return profiler_;
}

void CompilationSession::printTimingReport(std::ostream& out) const {
    if (!options_.showTiming) {
        return;
    }

    out << profiler_.report();
}

} // namespace portugol::core
