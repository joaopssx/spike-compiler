#ifndef SPIKE_ERROR_CATALOG_HPP
#define SPIKE_ERROR_CATALOG_HPP

#include <string_view>
#include <vector>

namespace spike {

// One "## E400 - title" entry of docs/errors.md, embedded at build time by
// cmake/embed_errors.cmake.
struct ErrorEntry {
    std::string_view code;
    std::string_view title;
    std::string_view message;
    std::string_view when;
    std::string_view fix;
    std::string_view example;
};

// Every entry, sorted by code.
const std::vector<ErrorEntry>& all_errors();

// Case-insensitive: "e400" finds E400. nullptr when the code is unknown.
const ErrorEntry* find_error(std::string_view code);

} // namespace spike

#endif // SPIKE_ERROR_CATALOG_HPP
