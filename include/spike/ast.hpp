#ifndef SPIKE_AST_HPP
#define SPIKE_AST_HPP

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace spike {

// Forward declarations — Expr e Stmt são "structs envelope" que carregam
// um std::variant. A indireção via unique_ptr permite recursão (BinaryExpr
// guarda ExprPtr left/right etc.) sem virtuais.
struct Expr;
struct Stmt;

using ExprPtr  = std::unique_ptr<Expr>;
using StmtPtr  = std::unique_ptr<Stmt>;
using StmtList = std::vector<StmtPtr>;

// ─── EXPRESSÕES ────────────────────────────────────────────────────────────

struct NumberExpr {
    double value = 0.0;
    bool   is_integer = true;  // true se não tem ponto decimal
    int line = 0, col = 0;
};

struct StringExpr {
    std::string value;  // conteúdo sem aspas, com escapes processados
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
    std::string op;   // "+", "-", "*", "/", "=", "<>", "<", ">", "<=", ">=", "e", "ou"
    ExprPtr left;
    ExprPtr right;
    int line = 0, col = 0;
};

struct UnaryExpr {
    std::string op;  // "nao", "-"
    ExprPtr operand;
    int line = 0, col = 0;
};

// Envelope: um Expr é uma das possibilidades acima.
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

// ─── STATEMENTS ────────────────────────────────────────────────────────────

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
};

struct SeStmt {
    ExprPtr    condition;
    StmtList   then_branch;
    StmtList   else_branch;  // vazio se não tem senao
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
    ExprPtr     step;   // nullptr = passo 1 implícito
    StmtList    body;
    int line = 0, col = 0;
};

struct RepitaStmt {
    StmtList body;
    ExprPtr  condition;  // a condição do "ate"
    int line = 0, col = 0;
};

// Envelope: um Stmt é uma das possibilidades acima.
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

// ─── DECLARAÇÕES ───────────────────────────────────────────────────────────

struct VarDecl {
    std::string name;
    std::string type;         // "inteiro", "real", "caractere", "logico"
    ExprPtr     initializer;  // nullptr se não tem valor inicial
    int line = 0, col = 0;
};

// ─── RAIZ DA AST ───────────────────────────────────────────────────────────

struct Program {
    std::string           name;     // nome do algoritmo
    std::vector<VarDecl>  globals;  // variáveis declaradas em var
    StmtList              body;     // instruções em inicio...fimalgoritmo
    int line = 1;
};

// ─── HELPERS DE CONSTRUÇÃO ────────────────────────────────────────────────
// Conveniências curtas pro parser e pra testes — evita std::make_unique<Expr>(
// Expr{...}) verbose nas calls. Não são parte do "spec da AST", só açúcar.

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
