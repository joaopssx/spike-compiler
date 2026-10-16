#ifndef SPIKE_AST_HPP
#define SPIKE_AST_HPP

#include "spike/ast_enums.hpp"

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace spike {

// Forward declarations - Expr e Stmt sao "structs envelope" que carregam
// um std::variant. A indirecao via unique_ptr permite recursao (BinaryExpr
// guarda ExprPtr left/right etc.) sem virtuais.
struct Expr;
struct Stmt;

struct SourcePos {
    int line = 0;
    int col = 0;
};

using ExprPtr  = std::unique_ptr<Expr>;
using StmtPtr  = std::unique_ptr<Stmt>;
using StmtList = std::vector<StmtPtr>;

// --- EXPRESSOES ------------------------------------------------------------

struct NumberExpr {
    double value = 0.0;
    bool   is_integer = true;  // true se nao tem ponto decimal
    int line = 0, col = 0;
};

struct StringExpr {
    std::string value;  // conteudo sem aspas, com escapes processados
    int line = 0, col = 0;
};

struct BoolExpr {
    bool value = false;  // verdadeiro / falso
    int line = 0, col = 0;
};

struct VarExpr {
    std::string name;  // nome do identificador
    int line = 0, col = 0;
};

struct BinaryExpr {
    BinaryOp op = BinaryOp::Add;
    ExprPtr left;
    ExprPtr right;
    int line = 0, col = 0;
};

struct UnaryExpr {
    UnaryOp op = UnaryOp::Neg;
    ExprPtr operand;
    int line = 0, col = 0;
};

// Envelope: um Expr e uma das possibilidades acima.
struct Expr {
    std::variant<
        NumberExpr,
        StringExpr,
        BoolExpr,
        VarExpr,
        BinaryExpr,
        UnaryExpr
    > node;
};

// --- STATEMENTS ------------------------------------------------------------

struct AssignStmt {
    std::string var_name;
    ExprPtr value;
    int line = 0, col = 0;
};

struct EscrevaStmt {
    std::vector<ExprPtr> args;
    bool newline = false;  // true = escreval, false = escreva
    int line = 0, col = 0;
};

struct LeiaStmt {
    std::vector<std::string> var_names;
    int line = 0, col = 0;
    std::vector<SourcePos> var_positions;
};

struct SeStmt {
    ExprPtr    condition;
    StmtList   then_branch;
    StmtList   else_branch;  // vazio se nao tem senao
    int line = 0, col = 0;
};

struct EnquantoStmt {
    ExprPtr  condition;
    StmtList body;
    int line = 0, col = 0;
};

struct ParaStmt {
    std::string var_name;
    ExprPtr     from;
    ExprPtr     to;
    ExprPtr     step;   // nullptr = passo 1 implicito
    StmtList    body;
    int line = 0, col = 0;
    SourcePos   var_pos;
};

struct RepitaStmt {
    StmtList body;
    ExprPtr  condition;  // a condicao do "ate"
    int line = 0, col = 0;
};

// Envelope: um Stmt e uma das possibilidades acima.
struct Stmt {
    std::variant<
        AssignStmt,
        EscrevaStmt,
        LeiaStmt,
        SeStmt,
        EnquantoStmt,
        ParaStmt,
        RepitaStmt
    > node;
};

// --- DECLARACOES -----------------------------------------------------------

struct VarDecl {
    std::string name;
    TypeKind    type = TypeKind::None;
    ExprPtr     initializer;  // nullptr se nao tem valor inicial
    int line = 0, col = 0;
};

struct ConstDecl {
    std::string name;
    ExprPtr     value;
    int line = 0, col = 0;
};

// --- RAIZ DA AST -----------------------------------------------------------

struct Program {
    std::string           name;     // nome do algoritmo
    std::vector<ConstDecl> consts;
    std::vector<VarDecl>  globals;  // variaveis declaradas em var
    StmtList              body;     // instrucoes em inicio...fimalgoritmo
    int line = 1;
};

// --- HELPERS DE CONSTRUCAO ------------------------------------------------
// Conveniencias curtas pro parser e pra testes - evita std::make_unique<Expr>(
// Expr{...}) verbose nas calls. Nao sao parte do "spec da AST", so acucar.

template <typename T>
inline ExprPtr make_expr(T&& alt) {
    return std::make_unique<Expr>(Expr{std::forward<T>(alt)});
}

template <typename T>
inline StmtPtr make_stmt(T&& alt) {
    return std::make_unique<Stmt>(Stmt{std::forward<T>(alt)});
}

} // namespace spike

#endif // SPIKE_AST_HPP
