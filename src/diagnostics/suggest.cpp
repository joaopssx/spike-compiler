#include "spike/suggest.hpp"
#include "diagnostics/utf8.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace spike {

namespace {

char32_t lower(char32_t c) {
    if (c >= U'A' && c <= U'Z') return c + 32;
    if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 32;
    return c;
}

std::u32string code_points(const std::string& text) {
    std::u32string out;
    for (std::size_t i = 0; i < text.size();) {
        const std::size_t length = utf8::sequence_length(text, i);
        if (length == 0) {
            out += static_cast<char32_t>(static_cast<unsigned char>(text[i]));
            ++i;
        } else {
            out += lower(utf8::decode(text, i, length));
            i += length;
        }
    }
    return out;
}

// Optimal string alignment distance, three rows at a time. Stops early once every value in a
// row is above `limit` and then returns limit + 1.
std::size_t distance(const std::u32string& a, const std::u32string& b, std::size_t limit) {
    const std::size_t cols = b.size() + 1;
    std::vector<std::size_t> before(cols), previous(cols), current(cols);
    for (std::size_t j = 0; j < cols; ++j) previous[j] = j;
    for (std::size_t i = 1; i <= a.size(); ++i) {
        current[0] = i;
        std::size_t row_min = current[0];
        for (std::size_t j = 1; j < cols; ++j) {
            const std::size_t cost = a[i - 1] == b[j - 1] ? 0 : 1;
            std::size_t best = std::min({previous[j] + 1, current[j - 1] + 1, previous[j - 1] + cost});
            if (i > 1 && j > 1 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1]) {
                best = std::min(best, before[j - 2] + 1);
            }
            current[j] = best;
            row_min = std::min(row_min, best);
        }
        if (row_min > limit) return limit + 1;
        before.swap(previous);
        previous.swap(current);
    }
    return previous[cols - 1];
}

std::size_t allowed_distance(std::size_t letters) {
    const std::size_t by_size = letters <= 4 ? 1 : letters <= 8 ? 2 : 3;
    return std::min(by_size, letters / 2);
}

} // namespace

std::size_t edit_distance(const std::string& a, const std::string& b) {
    const std::u32string left = code_points(a);
    const std::u32string right = code_points(b);
    return distance(left, right, std::max(left.size(), right.size()));
}

std::optional<std::string> suggest(const std::string& word,
                                   const std::vector<std::string>& candidates) {
    const std::u32string target = code_points(word);
    const std::size_t limit = allowed_distance(target.size());
    std::optional<std::string> best;
    std::size_t best_distance = limit + 1;
    std::size_t best_length = 0;
    for (const std::string& candidate : candidates) {
        const std::u32string other = code_points(candidate);
        const std::size_t gap =
            other.size() > target.size() ? other.size() - target.size() : target.size() - other.size();
        if (gap > limit) continue;
        const std::size_t d = distance(target, other, limit);
        if (d == 0 || d > limit) continue;
        const bool better = d < best_distance ||
                            (d == best_distance && other.size() < best_length) ||
                            (d == best_distance && other.size() == best_length && best &&
                             candidate < *best);
        if (better) {
            best = candidate;
            best_distance = d;
            best_length = other.size();
        }
    }
    return best;
}

std::string hint_with_suggestion(const std::string& word,
                                 const std::vector<std::string>& candidates,
                                 const std::string& fallback) {
    if (const std::optional<std::string> mistake = common_mistake(word)) return *mistake;
    if (const std::optional<std::string> close = suggest(word, candidates)) {
        std::string rest = fallback;
        if (!rest.empty() && rest[0] >= 'a' && rest[0] <= 'z') rest[0] = static_cast<char>(rest[0] - 32);
        return "você quis dizer '" + *close + "'? " + rest;
    }
    return fallback;
}

} // namespace spike
