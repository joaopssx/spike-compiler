#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <type_traits>
#include <variant>

namespace spike {

void SemanticAnalyzer::declare_consts() {
    for (const ConstDecl& decl : program_.consts) declare_const(decl);
}

std::optional<NameRef> SemanticAnalyzer::first_non_constant(const ExprPtr& expr) const {
    if (!expr) return std::nullopt;
    return std::visit(
        [this](const auto& node) -> std::optional<NameRef> {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, VarExpr>) {
                const Symbol* symbol = symbols_.lookup(node.name);
                if (symbol && symbol->is_const) return std::nullopt;
                return NameRef{node.name, node.line, node.col};
            } else if constexpr (std::is_same_v<Node, IndexExpr> ||
                                 std::is_same_v<Node, CallExpr>) {
                return NameRef{node.name, node.line, node.col};
            } else if constexpr (std::is_same_v<Node, BinaryExpr>) {
                if (auto left = first_non_constant(node.left)) return left;
                return first_non_constant(node.right);
            } else if constexpr (std::is_same_v<Node, UnaryExpr>) {
                return first_non_constant(node.operand);
            } else {
                return std::nullopt;
            }
        },
        expr->node);
}

bool SemanticAnalyzer::declared_anywhere(const std::string& name) const {
    const std::string key = symbol_key(name);
    for (const VarDecl& decl : program_.globals) {
        if (symbol_key(decl.name) == key) return true;
    }
    for (const ConstDecl& decl : program_.consts) {
        if (symbol_key(decl.name) == key) return true;
    }
    for (const ProcedureDecl& proc : program_.procedures) {
        if (symbol_key(proc.name) == key) return true;
    }
    return false;
}

void SemanticAnalyzer::declare_const(const ConstDecl& decl) {
    if (const Symbol* first = symbols_.lookup(decl.name)) {
        diags_.error("E701", "'" + decl.name + "' já foi declarada", decl.line, decl.col,
                     "a primeira declaração de '" + first->name + "' está na linha " +
                         std::to_string(first->line) + ". Apague uma das duas.");
        return;
    }

    TypeKind type = TypeKind::Invalid;
    if (const auto bad = first_non_constant(decl.value)) {
        if (declared_anywhere(bad->name)) {
            diags_.error("E741",
                         "o valor de uma constante precisa ser fixo (literais e outras constantes)",
                         bad->line, bad->col,
                         "'" + bad->name + "' não é uma constante declarada antes de '" +
                             decl.name + "'.");
        } else {
            resolve(bad->name, bad->line, bad->col);
        }
    } else if (decl.value) {
        type = check_expr(decl.value);
        if (type != TypeKind::Invalid) {
            const ConstEvaluation result = evaluate_constant(*decl.value, constants_);
            if (result.ok) {
                constants_[symbol_key(decl.name)] = result.value;
            } else {
                diags_.error("E742",
                             "não foi possível calcular a constante '" + decl.name + "': " +
                                 result.problem,
                             result.pos.line, result.pos.col,
                             "o valor de uma constante é calculado antes de o programa rodar.");
                type = TypeKind::Invalid;
            }
        }
    }
    symbols_.declare(decl.name, type, decl.line, decl.col, true);
}

} // namespace spike
