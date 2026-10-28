#ifndef SPIKE_SOURCE_VIEW_HPP
#define SPIKE_SOURCE_VIEW_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace spike::source_view {

constexpr std::size_t kTabWidth = 4;
constexpr std::size_t kLongLine = 100;
constexpr std::size_t kWindowWidth = 80;
inline constexpr const char* kEllipsis = "...";

// One entry per screen column: a character, or one of the spaces of a tab.
std::vector<std::string> display_cells(const std::string& line);

// Screen column (0-based) of source column `col` (1-based, in characters).
std::size_t caret_cell(const std::string& line, int col);

struct Window {
    std::size_t first = 0;
    std::size_t last = 0;
    bool whole = true;
};

Window window_for(const std::string& line, std::size_t caret);

std::string render(const std::string& line, const Window& window);

std::size_t caret_offset(std::size_t caret, const Window& window);

} // namespace spike::source_view

#endif // SPIKE_SOURCE_VIEW_HPP
