#include "spike/lexer.hpp"
#include "lexer/lexer_hints.hpp"

#include <cctype>
#include <string>
#include <unordered_map>

namespace spike {

namespace {

std::string to_lower_ascii(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

} // namespace

using namespace lexer_hints;

Token Lexer::scan_identifier_or_keyword() {
    const int start = current_;
    while (!is_at_end() && is_alphanumeric(peek())) {
        advance();
    }
    const std::string lexeme = source_.substr(
        static_cast<std::size_t>(start),
        static_cast<std::size_t>(current_ - start));

    const TokenType type = keyword_type(to_lower_ascii(lexeme));
    return make_token(type, lexeme);
}

Token Lexer::scan_number() {
    const int start = current_;
    while (!is_at_end() && is_digit(peek())) {
        advance();
    }

    if (peek() == '.' && is_digit(peek_next())) {
        advance();
        while (!is_at_end() && is_digit(peek())) {
            advance();
        }
    } else if (peek() == '.') {
        add_error("E003", "número malformado: '.' sem dígito decimal",
                  kE003Hint);
        advance();
    }

    if (peek() == '.' && is_digit(peek_next())) {
        add_error("E003", "número malformado: múltiplos pontos decimais",
                  kE003Hint);
        advance();
        while (!is_at_end() && is_digit(peek())) {
            advance();
        }
    }

    const std::string lexeme = source_.substr(
        static_cast<std::size_t>(start),
        static_cast<std::size_t>(current_ - start));
    return make_token(TokenType::NUMBER, lexeme);
}

Token Lexer::scan_string() {
    advance(); // consome a aspas de abertura

    std::string value;
    while (!is_at_end() && peek() != '"' && peek() != '\n') {
        char c = advance();
        if (c == '\\' && !is_at_end()) {
            const char next = advance();
            switch (next) {
                case '"':  value.push_back('"');  break;
                case '\\': value.push_back('\\'); break;
                case 'n':  value.push_back('\n'); break;
                case 't':  value.push_back('\t'); break;
                default:
                    value.push_back('\\');
                    value.push_back(next);
                    break;
            }
        } else {
            value.push_back(c);
        }
    }

    if (peek() != '"') {
        add_error("E002", "string não fechada", kE002Hint);
        return make_token(TokenType::STRING, value);
    }

    advance();
    return make_token(TokenType::STRING, value);
}

TokenType Lexer::keyword_type(const std::string& word) const {
    static const std::unordered_map<std::string, TokenType> kKeywords = {
        {"algoritmo",     TokenType::ALGORITMO},
        {"fimalgoritmo",  TokenType::FIM_ALGORITMO},
        {"inicio",        TokenType::INICIO},
        {"var",           TokenType::VAR},
        {"inteiro",       TokenType::INTEIRO},
        {"real",          TokenType::REAL},
        {"caractere",     TokenType::CARACTERE},
        {"caracter",      TokenType::CARACTERE},
        {"logico",        TokenType::LOGICO},
        {"se",            TokenType::SE},
        {"entao",         TokenType::ENTAO},
        {"senao",         TokenType::SENAO},
        {"fimse",         TokenType::FIM_SE},
        {"enquanto",      TokenType::ENQUANTO},
        {"faca",          TokenType::FACA},
        {"fimenquanto",   TokenType::FIM_ENQUANTO},
        {"para",          TokenType::PARA},
        {"de",            TokenType::DE},
        {"ate",           TokenType::ATE},
        {"passo",         TokenType::PASSO},
        {"fimpara",       TokenType::FIM_PARA},
        {"repita",        TokenType::REPITA},
        {"escreva",       TokenType::ESCREVA},
        {"escreval",      TokenType::ESCREVAL},
        {"leia",          TokenType::LEIA},
        {"verdadeiro",    TokenType::VERDADEIRO},
        {"falso",         TokenType::FALSO},
        {"e",             TokenType::E},
        {"ou",            TokenType::OU},
        {"nao",           TokenType::NAO},
        {"div",           TokenType::DIV},
        {"mod",           TokenType::MOD},
    };
    const auto it = kKeywords.find(word);
    if (it != kKeywords.end()) return it->second;
    return TokenType::IDENTIFIER;
}

} // namespace spike
