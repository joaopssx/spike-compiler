#include "codegen/codegen_internal.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace spike::codegen_detail {

namespace {

const char* const kHexDigits = "0123456789abcdef";

std::string octal_escape(unsigned char c) {
    std::string out = "\\";
    out += static_cast<char>('0' + ((c >> 6) & 7));
    out += static_cast<char>('0' + ((c >> 3) & 7));
    out += static_cast<char>('0' + (c & 7));
    return out;
}

} // namespace

std::string c_name(const std::string& name) {
    std::string out = "v_";
    for (unsigned char c : name) {
        if (c >= 0x80) {
            out += "_x";
            out += kHexDigits[c >> 4];
            out += kHexDigits[c & 15];
        } else if (c >= 'A' && c <= 'Z') {
            out += static_cast<char>(c - 'A' + 'a');
        } else {
            out += static_cast<char>(c);
        }
    }
    return out;
}

std::string int_literal(double value) {
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%lldLL", static_cast<long long>(value));
    return buffer;
}

std::string real_literal(double value) {
    char buffer[40];
    for (int precision = 1; precision <= 17; ++precision) {
        std::snprintf(buffer, sizeof buffer, "%.*g", precision, value);
        if (std::strtod(buffer, nullptr) == value) break;
    }
    std::string text = buffer;
    if (text.find_first_of(".en") == std::string::npos) text += ".0";
    return text;
}

std::string string_literal(const std::string& value) {
    std::string out = "\"";
    char previous = '\0';
    for (char ch : value) {
        const auto c = static_cast<unsigned char>(ch);
        if (ch == '"') {
            out += "\\\"";
        } else if (ch == '\\') {
            out += "\\\\";
        } else if (ch == '\n') {
            out += "\\n";
        } else if (ch == '\t') {
            out += "\\t";
        } else if (ch == '?' && previous == '?') {
            out += "\\?";
        } else if (c < 0x20 || c == 0x7f) {
            out += octal_escape(c);
        } else {
            out += ch;
        }
        previous = ch;
    }
    return out + "\"";
}

std::string comment_safe(const std::string& text) {
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '*' && i + 1 < text.size() && text[i + 1] == '/') {
            out += "* /";
            ++i;
        } else if (text[i] != '\n' && text[i] != '\r') {
            out += text[i];
        }
    }
    return out;
}

const char* c_type(TypeKind type) {
    switch (type) {
        case TypeKind::Integer: return "spike_int";
        case TypeKind::Real:    return "spike_real";
        case TypeKind::String:  return "spike_str";
        case TypeKind::Bool:    return "spike_bool";
        case TypeKind::None:
        case TypeKind::Invalid: return nullptr;
    }
    return nullptr;
}

const char* default_value(TypeKind type) {
    switch (type) {
        case TypeKind::Integer: return "0LL";
        case TypeKind::Real:    return "0.0";
        case TypeKind::String:  return "\"\"";
        case TypeKind::Bool:    return "0";
        case TypeKind::None:
        case TypeKind::Invalid: return nullptr;
    }
    return nullptr;
}

} // namespace spike::codegen_detail
