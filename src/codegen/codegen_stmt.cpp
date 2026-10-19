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
            } else if constexpr (std::is_same_v<Node, BreakStmt>) {
                return "interrompa";
            } else if constexpr (std::is_same_v<Node, EscolhaStmt>) {
                return "escolha";
            } else if constexpr (std::is_same_v<Node, CallStmt>) {
                return "chamada de procedimento";
            } else if constexpr (std::is_same_v<Node, ReturnStmt>) {
                return "retorne";
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

const char* read_function(TypeKind type) {
    switch (type) {
        case TypeKind::Integer: return "spike_read_int";
        case TypeKind::Real:    return "spike_read_real";
        case TypeKind::String:  return "spike_read_str";
        case TypeKind::Bool:    return "spike_read_bool";
        case TypeKind::None:
        case TypeKind::Invalid: return nullptr;
    }
    return nullptr;
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
    if (const auto* leia = std::get_if<LeiaStmt>(&stmt.node)) {
        emit_leia(*leia);
        return;
    }
    if (const auto* se = std::get_if<SeStmt>(&stmt.node)) {
        emit_se(*se);
        return;
    }
    if (const auto* enquanto = std::get_if<EnquantoStmt>(&stmt.node)) {
        emit_enquanto(*enquanto);
        return;
    }
    if (const auto* escolha = std::get_if<EscolhaStmt>(&stmt.node)) {
        emit_escolha(*escolha);
        return;
    }
    if (const auto* call = std::get_if<CallStmt>(&stmt.node)) {
        emit_call(*call);
        return;
    }
    if (const auto* ret = std::get_if<ReturnStmt>(&stmt.node)) {
        emit_return(*ret);
        return;
    }
    if (std::holds_alternative<BreakStmt>(stmt.node)) {
        out_.line("break;");
        return;
    }
    if (const auto* repita = std::get_if<RepitaStmt>(&stmt.node)) {
        emit_repita(*repita);
        return;
    }
    if (const auto* para = std::get_if<ParaStmt>(&stmt.node)) {
        emit_para(*para);
        return;
    }
    const SourcePos pos = stmt_pos(stmt);
    fail("E901",
         std::string("comando ainda não suportado pelo gerador: '") + command_name(stmt) + "'",
         pos.line, pos.col);
}

void Generator::emit_assign(const AssignStmt& stmt) {
    std::string name;
    const std::string target = lvalue(stmt.target, name);
    if (failed()) return;
    const auto it = var_types_.find(symbol_key(name));
    if (it == var_types_.end()) {
        fail("E900", "erro interno do gerador: variável '" + name + "' não declarada", stmt.line,
             stmt.col);
        return;
    }
    const std::string value = expr_as(stmt.value, it->second);
    if (failed()) return;
    out_.line(target + " = " + value + ";");
}

void Generator::emit_leia(const LeiaStmt& stmt) {
    for (const ExprPtr& target : stmt.targets) {
        std::string name;
        const std::string destination = lvalue(target, name);
        if (failed()) return;
        const int line = target ? std::visit([](const auto& node) { return node.line; }, target->node)
                                : stmt.line;
        const auto it = var_types_.find(symbol_key(name));
        const char* reader = it == var_types_.end() ? nullptr : read_function(it->second);
        if (!reader) {
            fail("E900", "erro interno do gerador: variável '" + name + "' sem tipo para o leia",
                 stmt.line, stmt.col);
            return;
        }
        out_.line(destination + " = " + reader + "(" + std::to_string(line) + ", " +
                  string_literal(name) + ");");
    }
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
