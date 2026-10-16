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

void SemanticAnalyzer::check_loop_body(const StmtList& stmts) {
    ++loop_depth_;
    check_block(stmts);
    --loop_depth_;
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
                for (const ExprPtr& target : node.targets) check_target(target);
            } else if constexpr (std::is_same_v<Node, SeStmt>) {
                check_condition(node.condition, "se");
                check_block(node.then_branch);
                check_block(node.else_branch);
            } else if constexpr (std::is_same_v<Node, EnquantoStmt>) {
                check_condition(node.condition, "enquanto");
                check_loop_body(node.body);
            } else if constexpr (std::is_same_v<Node, ParaStmt>) {
                check_para(node);
            } else if constexpr (std::is_same_v<Node, RepitaStmt>) {
                check_loop_body(node.body);
                check_condition(node.condition, "repita");
            } else if constexpr (std::is_same_v<Node, EscolhaStmt>) {
                check_escolha(node);
            } else if constexpr (std::is_same_v<Node, CallStmt>) {
                check_call(node);
            } else if constexpr (std::is_same_v<Node, BreakStmt>) {
                if (loop_depth_ == 0) {
                    diags_.error("E750", "'interrompa' só pode ser usado dentro de um laço",
                                 node.line, node.col,
                                 "use 'interrompa' dentro de enquanto, repita ou para.");
                }
            }
        },
        stmt.node);
}

} // namespace spike
