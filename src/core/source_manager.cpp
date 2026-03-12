#include "core/source_manager.hpp"

#include <algorithm>

#include "utils.hpp"

namespace portugol::core {

void SourceManager::load(const std::filesystem::path& path) {
    filePath_ = path;
    source_ = readFile(path);
    rebuildLineOffsets();
}

bool SourceManager::hasFile() const noexcept {
    return !filePath_.empty();
}

const std::filesystem::path& SourceManager::filePath() const noexcept {
    return filePath_;
}

std::string SourceManager::displayPath() const {
    return hasFile() ? filePath_.string() : std::string("<input>");
}

std::string_view SourceManager::source() const noexcept {
    return source_;
}

std::string_view SourceManager::lineText(std::size_t lineNumber) const {
    if (lineNumber == 0 || lineOffsets_.empty() || lineNumber > lineOffsets_.size()) {
        return {};
    }

    const std::size_t start = lineOffsets_[lineNumber - 1];
    std::size_t end = source_.size();
    if (lineNumber < lineOffsets_.size()) {
        end = lineOffsets_[lineNumber] - 1;
    }

    if (end > start && source_[end - 1] == '\r') {
        --end;
    }

    return std::string_view(source_).substr(start, end - start);
}

void SourceManager::rebuildLineOffsets() {
    lineOffsets_.clear();
    lineOffsets_.push_back(0);

    for (std::size_t index = 0; index < source_.size(); ++index) {
        if (source_[index] == '\n') {
            lineOffsets_.push_back(index + 1);
        }
    }

    if (!source_.empty() && source_.back() == '\n' && lineOffsets_.back() == source_.size()) {
        lineOffsets_.pop_back();
    }
}

} // namespace portugol::core
