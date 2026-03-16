#ifndef SPIKE_PARSER_HPP
#define SPIKE_PARSER_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "ast.hpp"
#include "token.hpp"

class ParserError : public std::runtime_error {
public:
    ParserError(std::string message, std::size_t line, std::size_t column);

    std::size_t line() const noexcept;
    std::size_t column() const noexcept;

private:
    std::size_t line_;
    std::size_t column_;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    Program parse_program();

private:
    bool is_at_end() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;
    const Token& consume(TokenType type, const std::string& message);

    std::vector<VarDecl> parse_var_declaration();
    Statement parse_statement();
    AssignStmt parse_assign_statement();
    WriteStmt parse_write_statement();
    Expression parse_expression();
    Expression parse_primary();
    std::string parse_type_name();
    [[noreturn]] void throw_error(const Token& token, const std::string& message) const;

    std::vector<Token> tokens_;
    std::size_t current_;
};

#endif
