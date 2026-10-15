#include "e2e_runner.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>

namespace spike_e2e {

std::optional<std::string> read_text(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return std::nullopt;
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string normalize(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        if (c != '\r') out += c;
    }
    return out;
}

std::string strip_ansi(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\x1b' && i + 1 < text.size() && text[i + 1] == '[') {
            i += 2;
            while (i < text.size() && !(text[i] >= '@' && text[i] <= '~')) ++i;
            continue;
        }
        out += text[i];
    }
    return out;
}

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start < text.size()) {
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return lines;
}

std::string first_difference(const std::string& expected, const std::string& actual) {
    const std::vector<std::string> want = split_lines(expected);
    const std::vector<std::string> got = split_lines(actual);
    const std::size_t count = std::max(want.size(), got.size());
    for (std::size_t i = 0; i < count; ++i) {
        const bool has_want = i < want.size();
        const bool has_got = i < got.size();
        if (has_want && has_got && want[i] == got[i]) continue;
        std::ostringstream report;
        report << "  first difference at line " << (i + 1) << "\n"
               << "    expected: " << (has_want ? "\"" + want[i] + "\"" : "(nothing)") << "\n"
               << "    got:      " << (has_got ? "\"" + got[i] + "\"" : "(nothing)") << "\n";
        return report.str();
    }
    const bool newline_expected = !expected.empty() && expected.back() == '\n';
    const bool newline_got = !actual.empty() && actual.back() == '\n';
    if (newline_expected != newline_got) {
        return std::string("  the last line ") +
               (newline_expected ? "should end with a line break\n" : "should not end with a line break\n");
    }
    return "";
}

std::optional<std::string> missing_lines_in_order(const std::string& expected,
                                                  const std::string& actual) {
    const std::vector<std::string> got = split_lines(actual);
    std::size_t position = 0;
    for (const std::string& line : split_lines(expected)) {
        if (line.empty()) continue;
        while (position < got.size() && got[position] != line) ++position;
        if (position == got.size()) return line;
        ++position;
    }
    return std::nullopt;
}

} // namespace spike_e2e
