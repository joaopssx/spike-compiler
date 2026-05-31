#ifndef SPIKE_PARSER_HPP
#define SPIKE_PARSER_HPP

#include "spike/ast.hpp"
#include "spike/diagnostics.hpp"
#include "spike/token.hpp"

#include <initializer_list>
#include <string>
#include <vector>

namespace spike {

class Parser {
public:
    Parser(std::vector<Token> tokens, const std::string& filename,
           const std::string& source);

    Program parse();

    // Entrada pública pra parsear uma única expressão. Usada por testes e,
    // eventualmente, pelos parsers de statement (PROMPTs seguintes).
    ExprPtr parse_expr();

    bool had_error() const;
    const Diagnostics& diagnostics() const;

private:
    std::vector<Token> tokens_;
    int current_;
    Diagnostics diags_;

    // Navegação
    Token& peek();
    Token& peek_next();
    Token& previous();
    Token& advance();
    bool   is_at_end() const;
    bool   check(TokenType t) const;
    bool   match(TokenType t);
    bool   match(std::initializer_list<TokenType> types);
    Token  expect(TokenType t, const std::string& error_msg,
                  const std::string& hint = "");

    // Parsing de alto nível
    Program              parse_program();
    std::vector<VarDecl> parse_var_section();

    // Parsing de statements (implementados nos próximos prompts)
    StmtList parse_stmt_list(TokenType stop_at);
    StmtPtr  parse_stmt();

    // Precedência crescente — cada nível chama o nível abaixo.
    ExprPtr parse_or();
    ExprPtr parse_and();
    ExprPtr parse_not();
    ExprPtr parse_comparison();
    ExprPtr parse_term();
    ExprPtr parse_factor();
    ExprPtr parse_unary();
    ExprPtr parse_primary();
};

} // namespace spike

#endif // SPIKE_PARSER_HPP
