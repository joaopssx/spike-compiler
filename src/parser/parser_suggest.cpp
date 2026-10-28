#include "spike/parser.hpp"
#include "spike/suggest.hpp"

#include <string>
#include <vector>

namespace spike {

namespace {

constexpr const char* kStatementHint =
    "instruções válidas: atribuição (<-), escreva, leia, se, enquanto, para.";

const char* closing_word(TokenType type) {
    switch (type) {
        case TokenType::FIM_SE: return "fimse";
        case TokenType::SENAO: return "senao";
        case TokenType::FIM_ENQUANTO: return "fimenquanto";
        case TokenType::FIM_PARA: return "fimpara";
        case TokenType::ATE: return "ate";
        case TokenType::FIM_ESCOLHA: return "fimescolha";
        case TokenType::CASO: return "caso";
        case TokenType::OUTROCASO: return "outrocaso";
        case TokenType::FIM_PROCEDIMENTO: return "fimprocedimento";
        case TokenType::FIM_FUNCAO: return "fimfuncao";
        case TokenType::FIM_ALGORITMO: return "fimalgoritmo";
        default: return nullptr;
    }
}

bool continues_a_statement(TokenType next) {
    return next == TokenType::ARROW || next == TokenType::ASSIGN || next == TokenType::LBRACKET ||
           next == TokenType::LPAREN || next == TokenType::EQUAL;
}

} // namespace

TokenType Parser::token_after_next() const {
    const std::size_t index = static_cast<std::size_t>(current_) + 2;
    return index < tokens_.size() ? tokens_[index].type : TokenType::END_OF_FILE;
}

std::string Parser::unknown_statement_hint(const Token& t) {
    if (t.type != TokenType::IDENTIFIER) return kStatementHint;
    const TokenType next = peek_next().type;
    if (next == TokenType::EQUAL) {
        return "para atribuir use '<-' (ou ':='): " + t.lexeme + " <- valor.";
    }
    if ((next == TokenType::PLUS || next == TokenType::MINUS) &&
        (token_after_next() == TokenType::EQUAL || token_after_next() == next)) {
        const std::string op = next == TokenType::PLUS ? "+" : "-";
        return "Portugol não tem '" + op + "=' nem '" + op + op + "': escreva " + t.lexeme +
               " <- " + t.lexeme + " " + op + " 1.";
    }
    std::vector<std::string> candidates = {"escreva", "escreval", "leia",       "se",
                                           "enquanto", "para",     "repita",     "interrompa",
                                           "retorne",  "escolha"};
    candidates.insert(candidates.end(), subprogram_names_.begin(), subprogram_names_.end());
    return hint_with_suggestion(t.lexeme, candidates, kStatementHint);
}

bool Parser::fix_misspelled_closer() {
    if (!check(TokenType::IDENTIFIER) || block_stops_.empty()) return false;
    if (continues_a_statement(peek_next().type)) return false;
    std::vector<std::string> words;
    std::vector<TokenType> types;
    for (TokenType type : block_stops_.back()) {
        if (const char* word = closing_word(type)) {
            words.emplace_back(word);
            types.push_back(type);
        }
    }
    const std::optional<std::string> close = suggest(peek().lexeme, words);
    if (!close) return false;
    Token& token = tokens_[static_cast<std::size_t>(current_)];
    report("E300", "instrução não reconhecida: '" + token.lexeme + "'", token.line, token.col,
           "você quis dizer '" + *close + "'?");
    for (std::size_t i = 0; i < words.size(); ++i) {
        if (words[i] == *close) token.type = types[i];
    }
    return true;
}

} // namespace spike
