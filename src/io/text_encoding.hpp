#ifndef SPIKE_TEXT_ENCODING_HPP
#define SPIKE_TEXT_ENCODING_HPP

#include <optional>
#include <string>

namespace spike::text_encoding {

bool strip_utf8_bom(std::string& text);

std::string normalize_newlines(const std::string& text);

std::string cp1252_to_utf8(const std::string& text);

std::optional<std::string> plausible_cp1252_to_utf8(const std::string& text);

} // namespace spike::text_encoding

#endif // SPIKE_TEXT_ENCODING_HPP
