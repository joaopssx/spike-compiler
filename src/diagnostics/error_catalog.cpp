#include "spike/error_catalog.hpp"

#include <cctype>
#include <string>

namespace spike {

const ErrorEntry* find_error(std::string_view code) {
    std::string wanted(code);
    for (char& c : wanted) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    for (const ErrorEntry& entry : all_errors()) {
        if (entry.code == wanted) return &entry;
    }
    return nullptr;
}

} // namespace spike
