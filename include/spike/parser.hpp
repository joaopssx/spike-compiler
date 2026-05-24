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

    // Parsing de statements (implementados nos próximos prompts).
    // Por enquanto parse_stmt_list só consome tokens até `stop_at` ou EOF.
    StmtList parse_stmt_list(TokenType stop_at);
    StmtPtr  parse_stmt();

    // Parsing de expressões (implementados nos próximos prompts).
    ExprPtr parse_expr();
};

} // namespace spike

#endif // SPIKE_PARSER_HPP
