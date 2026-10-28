#include "io/text_encoding.hpp"
#include "diagnostics/utf8.hpp"

#include <string>

namespace spike::text_encoding {

namespace {

constexpr char32_t kReplacement = 0xFFFD;

// Windows-1252 bytes 0x80..0x9F. The five bytes it leaves undefined become U+FFFD.
constexpr char32_t kHighControls[32] = {
    0x20AC, kReplacement, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, kReplacement, 0x017D, kReplacement,
    kReplacement, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, kReplacement, 0x017E, 0x0178,
};

char32_t cp1252_code_point(unsigned char byte) {
    if (byte >= 0x80 && byte <= 0x9F) return kHighControls[byte - 0x80];
    return byte;
}

void append_utf8(std::string& out, char32_t code) {
    if (code < 0x80) {
        out += static_cast<char>(code);
    } else if (code < 0x800) {
        out += static_cast<char>(0xC0 | (code >> 6));
        out += static_cast<char>(0x80 | (code & 0x3F));
    } else {
        out += static_cast<char>(0xE0 | (code >> 12));
        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (code & 0x3F));
    }
}

bool is_text_byte(unsigned char byte) {
    if (byte == '\t' || byte == '\n' || byte == '\r') return true;
    return byte >= 0x20 && byte != 0x7F;
}

bool has_utf8_multibyte_sequence(const std::string& text) {
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (static_cast<unsigned char>(text[i]) < 0x80) continue;
        if (utf8::sequence_length(text, i) > 1) return true;
    }
    return false;
}

} // namespace

bool strip_utf8_bom(std::string& text) {
    if (text.compare(0, 3, "\xEF\xBB\xBF") != 0) return false;
    text.erase(0, 3);
    return true;
}

std::string normalize_newlines(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\r') {
            out += text[i];
        } else {
            out += '\n';
            if (i + 1 < text.size() && text[i + 1] == '\n') ++i;
        }
    }
    return out;
}

std::string cp1252_to_utf8(const std::string& text) {
    std::string out;
    out.reserve(text.size() + text.size() / 4);
    for (char c : text) append_utf8(out, cp1252_code_point(static_cast<unsigned char>(c)));
    return out;
}

// A Windows-1252 file is accepted only when every byte is printable text and no byte pair
// already forms UTF-8: a file mixing both encodings is reported as invalid UTF-8 instead.
std::optional<std::string> plausible_cp1252_to_utf8(const std::string& text) {
    if (has_utf8_multibyte_sequence(text)) return std::nullopt;
    for (char c : text) {
        const auto byte = static_cast<unsigned char>(c);
        if (!is_text_byte(byte) || cp1252_code_point(byte) == kReplacement) return std::nullopt;
    }
    return cp1252_to_utf8(text);
}

} // namespace spike::text_encoding
