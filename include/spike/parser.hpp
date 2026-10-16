#ifndef SPIKE_PARSER_HPP
#define SPIKE_PARSER_HPP

#include "spike/ast.hpp"
#include "spike/diagnostics.hpp"
#include "spike/token.hpp"

#include <initializer_list>
#include <string>
#include <vector>

namespace spike
{

    class Parser
    {
    public:
        Parser(std::vector<Token> tokens, const std::string &filename,
               const std::string &source);

        Program parse();

        // Entrada publica pra parsear uma unica expressao. Usada pelos testes e
        // pelos parsers de statement.
        ExprPtr parse_expr();

        bool had_error() const;
        const Diagnostics &diagnostics() const;

    private:
        std::vector<Token> tokens_;
        int current_;
        Diagnostics diags_;

        // Navegacao
        Token &peek();
        Token &peek_next();
        Token &previous();
        Token &advance();
        bool is_at_end() const;
        bool check(TokenType t) const;
        bool match(TokenType t);
        bool match(std::initializer_list<TokenType> types);
        Token expect(TokenType t, const std::string &error_msg,
                     const std::string &hint = "");

        // Parsing de alto nivel
        Program parse_program();
        std::vector<VarDecl> parse_var_section();
    std::vector<ConstDecl> parse_const_section();
    void skip_line(int line);
        ProcedureDecl parse_procedure();
        bool parse_params(std::vector<Param>& params, const std::string& procedure);
        StmtPtr parse_call();
        StmtPtr reject_late_procedure();

        // Parsing de statements
        StmtList parse_stmt_list(TokenType stop_at);
        StmtList parse_stmt_list(std::initializer_list<TokenType> stop_tokens);
        StmtPtr parse_stmt();
        StmtPtr parse_assign();
        StmtPtr parse_escreva(bool newline);
        StmtPtr parse_leia();
        StmtPtr parse_se();
        StmtPtr parse_enquanto();
        StmtPtr parse_para();
        StmtPtr parse_repita();
        StmtPtr parse_escolha();
        ExprPtr parse_name_reference(const Token& name);
        bool parse_array_type(std::vector<ArrayDim>& dims);
        bool parse_array_bound(ArrayBound& bound);
        bool parse_case_labels(CaseClause& clause);

        // Precedencia crescente - cada nivel chama o nivel abaixo.
        ExprPtr parse_or();
        ExprPtr parse_and();
        ExprPtr parse_not();
        ExprPtr parse_comparison();
        ExprPtr parse_term();
        ExprPtr parse_factor();
        ExprPtr parse_unary();
        ExprPtr parse_power();
        ExprPtr parse_primary();
    };

} // namespace spike

#endif // SPIKE_PARSER_HPP
