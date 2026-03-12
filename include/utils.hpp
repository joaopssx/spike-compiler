#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>

#include "token.hpp"

namespace portugol {

class CompilerError : public std::runtime_error {
public:
    CompilerError(std::string stage, std::string message, SourceLocation location);

    const std::string& stage() const noexcept;
    const std::string& message() const noexcept;
    const SourceLocation& location() const noexcept;

private:
    std::string stage_;
    std::string message_;
    SourceLocation location_;
};

std::string readFile(const std::filesystem::path& path);
void writeFile(const std::filesystem::path& path, const std::string& content);
std::string formatDiagnostic(const std::string& fileName, const std::string& source, const CompilerError& error);

} // namespace portugol
