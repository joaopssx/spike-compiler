#include "token_json.hpp"

#include <sstream>

namespace portugol {

namespace {

std::string jsonEscape(std::string_view text) {
    std::string escaped;
    for (const char ch : text) {
        switch (ch) {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\n':
            escaped += "\\n";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

} // namespace

std::string tokensToJson(const std::vector<Token>& tokens) {
    std::ostringstream out;
    out << "[\n";
    for (std::size_t index = 0; index < tokens.size(); ++index) {
        const auto& token = tokens[index];
        out << "  {\n";
        out << "    \"type\": \"" << tokenTypeName(token.type) << "\",\n";
        out << "    \"lexeme\": \"" << jsonEscape(token.lexeme) << "\",\n";
        out << "    \"location\": {\n";
        out << "      \"line\": " << token.location.line << ",\n";
        out << "      \"column\": " << token.location.column << ",\n";
        out << "      \"offset\": " << token.location.offset << "\n";
        out << "    }\n";
        out << "  }";
        if (index + 1 < tokens.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return out.str();
}

} // namespace portugol
