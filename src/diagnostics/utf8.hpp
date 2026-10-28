#ifndef SPIKE_UTF8_HPP
#define SPIKE_UTF8_HPP

#include <cstddef>
#include <string>

namespace spike::utf8 {

inline bool is_continuation(char c) {
    return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
}

// Bytes of the valid sequence that starts at `pos`, or 0 if it is not valid UTF-8.
std::size_t sequence_length(const std::string& text, std::size_t pos);

char32_t decode(const std::string& text, std::size_t pos, std::size_t length);

// Offset of the first byte that is not valid UTF-8, or npos.
std::size_t first_invalid(const std::string& text);

bool is_latin_letter(char32_t code_point);

// Latin-1 and Latin Extended-A letters become ASCII (an accented "media" loses its accent).
std::string strip_accents(const std::string& word);

} // namespace spike::utf8

#endif // SPIKE_UTF8_HPP
