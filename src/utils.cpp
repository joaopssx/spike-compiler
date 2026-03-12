#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace portugol {

CompilerError::CompilerError(std::string stage, std::string message, SourceLocation location)
    : std::runtime_error(stage + ": " + message)
    , stage_(std::move(stage))
    , message_(std::move(message))
    , location_(location) {
}

const std::string& CompilerError::stage() const noexcept {
    return stage_;
}

const std::string& CompilerError::message() const noexcept {
    return message_;
}

const SourceLocation& CompilerError::location() const noexcept {
    return location_;
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("nao foi possivel abrir o arquivo '" + path.string() + "'");
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void writeFile(const std::filesystem::path& path, const std::string& content) {
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("nao foi possivel escrever o arquivo '" + path.string() + "'");
    }

    output << content;
}

std::string formatDiagnostic(const std::string& fileName, const std::string& source, const CompilerError& error) {
    std::vector<std::string> lines;
    std::string currentLine;

    for (const char ch : source) {
        if (ch == '\n') {
            lines.push_back(currentLine);
            currentLine.clear();
            continue;
        }
        if (ch != '\r') {
            currentLine.push_back(ch);
        }
    }
    lines.push_back(currentLine);

    const std::size_t lineIndex = error.location().line == 0 ? 0 : error.location().line - 1;
    const std::string lineText = lineIndex < lines.size() ? lines[lineIndex] : "";
    const std::size_t caretColumn = error.location().column > 0 ? error.location().column : 1;

    std::ostringstream out;
    out << fileName << ":" << error.location().line << ":" << error.location().column << ": " << error.stage() << " error: " << error.message() << "\n";
    out << lineText << "\n";
    out << std::string(caretColumn > 1 ? caretColumn - 1 : 0, ' ') << "^\n";
    return out.str();
}

} // namespace portugol
