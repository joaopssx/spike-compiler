#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace portugol::core {

class SourceManager {
public:
    void load(const std::filesystem::path& path);

    bool hasFile() const noexcept;
    const std::filesystem::path& filePath() const noexcept;
    std::string displayPath() const;
    std::string_view source() const noexcept;
    std::string_view lineText(std::size_t lineNumber) const;

private:
    void rebuildLineOffsets();

    std::filesystem::path filePath_;
    std::string source_;
    std::vector<std::size_t> lineOffsets_;
};

} // namespace portugol::core
