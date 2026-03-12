#pragma once

#include <iosfwd>
#include <string>

#include "core/source_manager.hpp"
#include "utils.hpp"

namespace portugol::core {

enum class DiagnosticLevel {
    Error,
    Warning,
    Note,
    Help,
    Trace,
};

struct Diagnostic {
    DiagnosticLevel level {DiagnosticLevel::Error};
    std::string stage;
    std::string message;
    SourceLocation location;
};

class DiagnosticEngine {
public:
    explicit DiagnosticEngine(const SourceManager* sourceManager = nullptr);

    void setSourceManager(const SourceManager* sourceManager) noexcept;
    void setTraceEnabled(bool enabled) noexcept;

    std::string render(const Diagnostic& diagnostic) const;
    void emit(const Diagnostic& diagnostic, std::ostream& out) const;
    void emitCompilerError(const CompilerError& error, std::ostream& out) const;
    void trace(const std::string& stage, const std::string& message, std::ostream& out) const;

private:
    static std::string_view levelName(DiagnosticLevel level);

    const SourceManager* sourceManager_ {nullptr};
    bool traceEnabled_ {false};
};

} // namespace portugol::core
