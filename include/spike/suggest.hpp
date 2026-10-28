#ifndef SPIKE_SUGGEST_HPP
#define SPIKE_SUGGEST_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace spike {

// Damerau-Levenshtein distance (with swaps of neighbouring letters), ignoring case and
// counting UTF-8 characters, not bytes.
std::size_t edit_distance(const std::string& a, const std::string& b);

// The closest candidate within the allowed distance for `word`, or nullopt.
std::optional<std::string> suggest(const std::string& word,
                                   const std::vector<std::string>& candidates);

// Hint for words that come from C, Python or Java ("print", "if", "true"), or nullopt.
std::optional<std::string> common_mistake(const std::string& word);

// The table hint when `word` is a common mistake; otherwise "você quis dizer 'x'?" followed by
// `fallback` when a candidate is close; otherwise `fallback`.
std::string hint_with_suggestion(const std::string& word,
                                 const std::vector<std::string>& candidates,
                                 const std::string& fallback);

} // namespace spike

#endif // SPIKE_SUGGEST_HPP
