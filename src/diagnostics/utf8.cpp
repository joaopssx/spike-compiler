#include "diagnostics/utf8.hpp"

namespace spike::utf8 {

namespace {

constexpr char32_t kFirstLatinLetter = 0xC0;
constexpr char32_t kLastLatinLetter = 0x17F;

const char* const kAsciiOfLatin[] = {
    "A", "A", "A", "A", "A", "A", "AE", "C", "E", "E", "E", "E", "I", "I", "I", "I",
    "D", "N", "O", "O", "O", "O", "O", "", "O", "U", "U", "U", "U", "Y", "Th", "ss",
    "a", "a", "a", "a", "a", "a", "ae", "c", "e", "e", "e", "e", "i", "i", "i", "i",
    "d", "n", "o", "o", "o", "o", "o", "", "o", "u", "u", "u", "u", "y", "th", "y",
    "A", "a", "A", "a", "A", "a", "C", "c", "C", "c", "C", "c", "C", "c", "D", "d",
    "D", "d", "E", "e", "E", "e", "E", "e", "E", "e", "E", "e", "G", "g", "G", "g",
    "G", "g", "G", "g", "H", "h", "H", "h", "I", "i", "I", "i", "I", "i", "I", "i",
    "I", "i", "IJ", "ij", "J", "j", "K", "k", "k", "L", "l", "L", "l", "L", "l", "L",
    "l", "L", "l", "N", "n", "N", "n", "N", "n", "n", "N", "n", "O", "o", "O", "o",
    "O", "o", "OE", "oe", "R", "r", "R", "r", "R", "r", "S", "s", "S", "s", "S", "s",
    "S", "s", "T", "t", "T", "t", "T", "t", "U", "u", "U", "u", "U", "u", "U", "u",
    "U", "u", "U", "u", "W", "w", "Y", "y", "Y", "Z", "z", "Z", "z", "Z", "z", "s",
};

static_assert(sizeof(kAsciiOfLatin) / sizeof(kAsciiOfLatin[0]) ==
                  kLastLatinLetter - kFirstLatinLetter + 1,
              "one entry per code point");

unsigned char byte_at(const std::string& text, std::size_t pos) {
    return static_cast<unsigned char>(text[pos]);
}

} // namespace

std::size_t sequence_length(const std::string& text, std::size_t pos) {
    const unsigned char lead = byte_at(text, pos);
    if (lead < 0x80) return 1;
    std::size_t length = 0;
    unsigned char min_second = 0x80;
    unsigned char max_second = 0xBF;
    if (lead >= 0xC2 && lead <= 0xDF) {
        length = 2;
    } else if (lead >= 0xE0 && lead <= 0xEF) {
        length = 3;
        if (lead == 0xE0) min_second = 0xA0;
        if (lead == 0xED) max_second = 0x9F;
    } else if (lead >= 0xF0 && lead <= 0xF4) {
        length = 4;
        if (lead == 0xF0) min_second = 0x90;
        if (lead == 0xF4) max_second = 0x8F;
    } else {
        return 0;
    }
    if (pos + length > text.size()) return 0;
    const unsigned char second = byte_at(text, pos + 1);
    if (second < min_second || second > max_second) return 0;
    for (std::size_t i = 2; i < length; ++i) {
        if (!is_continuation(text[pos + i])) return 0;
    }
    return length;
}

char32_t decode(const std::string& text, std::size_t pos, std::size_t length) {
    static const unsigned char kLeadMask[] = {0, 0x7F, 0x1F, 0x0F, 0x07};
    char32_t code_point = byte_at(text, pos) & kLeadMask[length];
    for (std::size_t i = 1; i < length; ++i) {
        code_point = (code_point << 6) | (byte_at(text, pos + i) & 0x3F);
    }
    return code_point;
}

std::size_t first_invalid(const std::string& text) {
    std::size_t pos = 0;
    while (pos < text.size()) {
        const std::size_t length = sequence_length(text, pos);
        if (length == 0) return pos;
        pos += length;
    }
    return std::string::npos;
}

bool is_latin_letter(char32_t code_point) {
    return code_point >= kFirstLatinLetter && code_point <= kLastLatinLetter &&
           code_point != 0xD7 && code_point != 0xF7;
}

std::string strip_accents(const std::string& word) {
    std::string out;
    std::size_t pos = 0;
    while (pos < word.size()) {
        std::size_t length = sequence_length(word, pos);
        if (length == 0) length = 1;
        const char32_t code_point = length == 1 ? byte_at(word, pos) : decode(word, pos, length);
        if (is_latin_letter(code_point)) {
            out += kAsciiOfLatin[code_point - kFirstLatinLetter];
        } else {
            out.append(word, pos, length);
        }
        pos += length;
    }
    return out;
}

} // namespace spike::utf8
