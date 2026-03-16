#include "parser.hpp"

#include <type_traits>
#include <utility>

ParserError::ParserError(std::string message, const std::size_t line, const std::size_t column)
    : std::runtime_error(std::move(message)), line_(line), column_(column) {}

std::size_t ParserError::line() const noexcept {
    return line_;
}

std::size_t ParserError::column() const noexcept {
    return column_;
}

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), current_(0) {}

Program Parser::parse_program() {
    Program program;

    consume(TokenType::KeywordAlgoritmo, "expected 'algoritmo' at program start");

    if (check(TokenType::Identifier)) {
        program.name = advance().lexeme;
    }

    if (match(TokenType::KeywordVar)) {
        while (check(TokenType::Identifier)) {
            std::vector<VarDecl> declarations = parse_var_declaration();
            program.variables.insert(program.variables.end(), declarations.begin(),
                                     declarations.end());
        }
    }

    consume(TokenType::KeywordInicio, "expected 'inicio' before statements");

    while (!check(TokenType::KeywordFimAlgoritmo) && !is_at_end()) {
        program.statements.push_back(parse_statement());
    }

    consume(TokenType::KeywordFimAlgoritmo, "expected 'fimalgoritmo' at end of program");
    consume(TokenType::EndOfFile, "expected end of file after 'fimalgoritmo'");

    return program;
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::EndOfFile;
}

bool Parser::check(const TokenType type) const {
    if (is_at_end()) {
        return type == TokenType::EndOfFile;
    }

    return peek().type == type;
}

bool Parser::match(const TokenType type) {
    if (!check(type)) {
        return false;
    }

    advance();
    return true;
}

const Token& Parser::advance() {
    if (!is_at_end()) {
        ++current_;
        return tokens_[current_ - 1];
    }

    return tokens_[current_];
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token& Parser::consume(const TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }

    throw_error(peek(), message);
}

std::vector<VarDecl> Parser::parse_var_declaration() {
    std::vector<std::string> names;
    names.push_back(consume(TokenType::Identifier, "expected variable name").lexeme);

    while (match(TokenType::Comma)) {
        names.push_back(consume(TokenType::Identifier, "expected variable name after ','").lexeme);
    }

    consume(TokenType::Colon, "expected ':' after variable name");
    const std::string type_name = parse_type_name();

    std::vector<VarDecl> declarations;
    declarations.reserve(names.size());

    for (const std::string& name : names) {
        declarations.push_back(VarDecl{name, type_name});
    }

    return declarations;
}

Statement Parser::parse_statement() {
    if (check(TokenType::Identifier)) {
        return parse_assign_statement();
    }

    if (check(TokenType::KeywordEscreva)) {
        return parse_write_statement();
    }

    throw_error(peek(), "expected a statement");
}

AssignStmt Parser::parse_assign_statement() {
    const std::string target = consume(TokenType::Identifier, "expected assignment target").lexeme;
    consume(TokenType::Assign, "expected '<-' after assignment target");

    return AssignStmt{target, parse_expression()};
}

WriteStmt Parser::parse_write_statement() {
    consume(TokenType::KeywordEscreva, "expected 'escreva'");
    consume(TokenType::LeftParen, "expected '(' after 'escreva'");
    const Expression value = parse_expression();
    consume(TokenType::RightParen, "expected ')' after write expression");

    return WriteStmt{value};
}

Expression Parser::parse_expression() {
    return parse_primary();
}

Expression Parser::parse_primary() {
    if (match(TokenType::Identifier)) {
        return IdentifierExpr{previous().lexeme};
    }

    if (match(TokenType::IntegerLiteral)) {
        return IntegerLiteralExpr{previous().lexeme};
    }

    if (match(TokenType::RealLiteral)) {
        return RealLiteralExpr{previous().lexeme};
    }

    if (match(TokenType::StringLiteral)) {
        return StringLiteralExpr{previous().lexeme};
    }

    throw_error(peek(), "expected an expression");
}

std::string Parser::parse_type_name() {
    if (match(TokenType::KeywordInteiro)) {
        return previous().lexeme;
    }

    if (match(TokenType::KeywordReal)) {
        return previous().lexeme;
    }

    if (match(TokenType::KeywordTexto)) {
        return previous().lexeme;
    }

    throw_error(peek(), "expected a type name");
}

[[noreturn]] void Parser::throw_error(const Token& token, const std::string& message) const {
    throw ParserError(message, token.line, token.column);
}
