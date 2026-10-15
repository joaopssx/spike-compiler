#include "codegen/codegen_internal.hpp"
#include "spike/symbol_table.hpp"

#include <string>
#include <type_traits>
#include <variant>

namespace spike::codegen_detail {

namespace {

const char* command_name(const Stmt& stmt) {
    return std::visit(
        [](const auto& node) -> const char* {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, EscrevaStmt>) {
                return node.newline ? "escreval" : "escreva";
            } else if constexpr (std::is_same_v<Node, LeiaStmt>) {
                return "leia";
            } else if constexpr (std::is_same_v<Node, SeStmt>) {
                return "se";
            } else if constexpr (std::is_same_v<Node, EnquantoStmt>) {
                return "enquanto";
            } else if constexpr (std::is_same_v<Node, ParaStmt>) {
                return "para";
            } else if constexpr (std::is_same_v<Node, RepitaStmt>) {
                return "repita";
            } else {
                return "atribuição";
            }
        },
        stmt.node);
}

const char* write_function(TypeKind type) {
    switch (type) {
        case TypeKind::Integer: return "spike_write_int";
        case TypeKind::Real:    return "spike_write_real";
        case TypeKind::String:  return "spike_write_str";
        case TypeKind::Bool:    return "spike_write_bool";
        case TypeKind::None:
        case TypeKind::Invalid: return "";
    }
    return "";
}

SourcePos stmt_pos(const Stmt& stmt) {
    return std::visit([](const auto& node) { return SourcePos{node.line, node.col}; },
                      stmt.node);
}

} // namespace

void Generator::emit_block(const StmtList& stmts) {
    for (const StmtPtr& stmt : stmts) {
        if (failed()) return;
        if (stmt) emit_stmt(*stmt);
    }
}

void Generator::emit_stmt(const Stmt& stmt) {
    if (const auto* assign = std::get_if<AssignStmt>(&stmt.node)) {
        emit_assign(*assign);
        return;
    }
    if (const auto* escreva = std::get_if<EscrevaStmt>(&stmt.node)) {
        emit_escreva(*escreva);
        return;
    }
    const SourcePos pos = stmt_pos(stmt);
    fail("E901",
         std::string("comando ainda não suportado pelo gerador: '") + command_name(stmt) + "'",
         pos.line, pos.col);
}

void Generator::emit_assign(const AssignStmt& stmt) {
    const auto it = var_types_.find(symbol_key(stmt.var_name));
    if (it == var_types_.end()) {
        fail("E900", "erro interno do gerador: variável '" + stmt.var_name + "' não declarada",
             stmt.line, stmt.col);
        return;
    }
    const std::string value = expr_as(stmt.value, it->second);
    if (failed()) return;
    out_.line(c_name(stmt.var_name) + " = " + value + ";");
}

void Generator::emit_escreva(const EscrevaStmt& stmt) {
    for (const ExprPtr& arg : stmt.args) {
        if (!arg) continue;
        const std::string value = expr(arg);
        const char* writer = write_function(type_of(*arg));
        if (failed()) return;
        out_.line(std::string(writer) + "(" + value + ");");
    }
    if (stmt.newline) out_.line("spike_write_newline();");
}

} // namespace spike::codegen_detail
