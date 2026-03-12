#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "token.hpp"

namespace portugol {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    Program parseProgram();

private:
    const Token& current() const;
    const Token& previous() const;
    bool isAtEnd() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& message);

    std::vector<std::unique_ptr<Stmt>> parseVarBlock();
    std::unique_ptr<Stmt> parseDeclaration();
    std::vector<std::string> parseIdentifierList();
    ValueType parseType();

    std::vector<std::unique_ptr<Stmt>> parseStatementListUntil(const std::vector<TokenType>& endTokens);
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseAssignment();
    std::unique_ptr<Stmt> parseWrite();
    std::unique_ptr<Stmt> parseRead();
    std::unique_ptr<Stmt> parseIf();
    std::unique_ptr<Stmt> parseWhile();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();

    const std::vector<Token>& tokens_;
    std::size_t pos_ {0};
};

} // namespace portugol
