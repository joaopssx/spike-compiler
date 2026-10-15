#include "spike/semantic.hpp"

#include <cstddef>
#include <type_traits>
#include <variant>

namespace spike {

void SemanticAnalyzer::check_block(const StmtList& stmts) {
    for (const StmtPtr& stmt : stmts) {
        if (stmt) check_stmt(*stmt);
    }
}

void SemanticAnalyzer::check_stmt(const Stmt& stmt) {
    std::visit(
        [this](const auto& node) {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, AssignStmt>) {
                check_assign(node);
            } else if constexpr (std::is_same_v<Node, EscrevaStmt>) {
                for (const ExprPtr& arg : node.args) check_expr(arg);
            } else if constexpr (std::is_same_v<Node, LeiaStmt>) {
                for (std::size_t i = 0; i < node.var_names.size(); ++i) {
                    const bool has_pos = i < node.var_positions.size();
                    const int line = has_pos ? node.var_positions[i].line : node.line;
                    const int col = has_pos ? node.var_positions[i].col : node.col;
                    mark_assigned(node.var_names[i], line, col);
                }
            } else if constexpr (std::is_same_v<Node, SeStmt>) {
                check_condition(node.condition, "se");
                check_block(node.then_branch);
                check_block(node.else_branch);
            } else if constexpr (std::is_same_v<Node, EnquantoStmt>) {
                check_condition(node.condition, "enquanto");
                check_block(node.body);
            } else if constexpr (std::is_same_v<Node, ParaStmt>) {
                check_para(node);
            } else if constexpr (std::is_same_v<Node, RepitaStmt>) {
                check_block(node.body);
                check_condition(node.condition, "repita");
            }
        },
        stmt.node);
}

} // namespace spike
