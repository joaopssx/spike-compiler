#include "core/diagnostics.hpp"

#include <iomanip>
#include <sstream>

namespace portugol::core {

DiagnosticEngine::DiagnosticEngine(const SourceManager* sourceManager)
    : sourceManager_(sourceManager) {
}

void DiagnosticEngine::setSourceManager(const SourceManager* sourceManager) noexcept {
    sourceManager_ = sourceManager;
}

void DiagnosticEngine::setTraceEnabled(bool enabled) noexcept {
    traceEnabled_ = enabled;
}

std::string DiagnosticEngine::render(const Diagnostic& diagnostic) const {
    std::ostringstream out;
    out << levelName(diagnostic.level);
    if (!diagnostic.stage.empty()) {
        out << "[" << diagnostic.stage << "]";
    }
    out << ": " << diagnostic.message << "\n";

    if (sourceManager_ == nullptr || !sourceManager_->hasFile() || diagnostic.location.line == 0) {
        return out.str();
    }

    const std::string lineNumberText = std::to_string(diagnostic.location.line);
    const std::string_view lineText = sourceManager_->lineText(diagnostic.location.line);
    const std::size_t gutterWidth = lineNumberText.size();
    const std::size_t caretColumn = diagnostic.location.column > 0 ? diagnostic.location.column : 1;

    out << " --> " << sourceManager_->displayPath() << ":" << diagnostic.location.line << ":" << diagnostic.location.column << "\n";
    out << std::string(gutterWidth, ' ') << " |\n";
    out << std::setw(static_cast<int>(gutterWidth)) << lineNumberText << " | " << lineText << "\n";
    out << std::string(gutterWidth, ' ') << " | " << std::string(caretColumn > 1 ? caretColumn - 1 : 0, ' ') << "^\n";
    return out.str();
}

void DiagnosticEngine::emit(const Diagnostic& diagnostic, std::ostream& out) const {
    out << render(diagnostic);
}

void DiagnosticEngine::emitCompilerError(const CompilerError& error, std::ostream& out) const {
    emit(Diagnostic {DiagnosticLevel::Error, error.stage(), error.message(), error.location()}, out);
}

void DiagnosticEngine::trace(const std::string& stage, const std::string& message, std::ostream& out) const {
    if (!traceEnabled_) {
        return;
    }

    out << "[trace][" << stage << "] " << message << "\n";
}

std::string_view DiagnosticEngine::levelName(DiagnosticLevel level) {
    switch (level) {
    case DiagnosticLevel::Error:
        return "error";
    case DiagnosticLevel::Warning:
        return "warning";
    case DiagnosticLevel::Note:
        return "note";
    case DiagnosticLevel::Help:
        return "help";
    case DiagnosticLevel::Trace:
        return "trace";
    }

    return "diagnostic";
}

} // namespace portugol::core
