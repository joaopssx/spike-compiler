#include "diagnostics/source_view.hpp"

#include "diagnostics/utf8.hpp"

#include <algorithm>
#include <cstring>

namespace spike::source_view {

std::vector<std::string> display_cells(const std::string& line) {
    std::vector<std::string> cells;
    std::size_t pos = 0;
    while (pos < line.size()) {
        if (line[pos] == '\t') {
            cells.insert(cells.end(), kTabWidth, " ");
            ++pos;
            continue;
        }
        const std::size_t length = utf8::sequence_length(line, pos);
        if (length == 0) {
            cells.emplace_back("?");
            ++pos;
            continue;
        }
        cells.push_back(line.substr(pos, length));
        pos += length;
    }
    return cells;
}

std::size_t caret_cell(const std::string& line, int col) {
    std::size_t cell = 0;
    int current = 1;
    std::size_t pos = 0;
    while (current < col && pos < line.size()) {
        if (line[pos] == '\t') {
            cell += kTabWidth;
            ++pos;
        } else {
            const std::size_t length = utf8::sequence_length(line, pos);
            cell += 1;
            pos += length == 0 ? 1 : length;
        }
        ++current;
    }
    if (current < col) cell += static_cast<std::size_t>(col - current);
    return cell;
}

Window window_for(const std::string& line, std::size_t caret) {
    const std::size_t size = display_cells(line).size();
    Window window;
    window.last = size;
    if (size <= kLongLine) return window;
    const std::size_t half = kWindowWidth / 2;
    window.whole = false;
    window.first = caret > half ? caret - half : 0;
    window.first = std::min(window.first, size - kWindowWidth);
    window.last = window.first + kWindowWidth;
    return window;
}

std::string render(const std::string& line, const Window& window) {
    const std::vector<std::string> cells = display_cells(line);
    std::string out;
    if (window.whole) {
        for (const std::string& cell : cells) out += cell;
        return out;
    }
    if (window.first > 0) out += kEllipsis;
    const std::size_t last = std::min(window.last, cells.size());
    for (std::size_t i = window.first; i < last; ++i) out += cells[i];
    if (cells.size() > window.last) out += kEllipsis;
    return out;
}

std::size_t caret_offset(std::size_t caret, const Window& window) {
    if (window.whole || window.first == 0) return caret;
    return caret - window.first + std::strlen(kEllipsis);
}

} // namespace spike::source_view
