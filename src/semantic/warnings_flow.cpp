#include "semantic/warnings_internal.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <type_traits>
#include <variant>

namespace spike::warnings_detail {

namespace {

using semantic_detail::expr_start;

bool is_literal_zero(const Expr& expr) {
    if (const auto* number = std::get_if<NumberExpr>(&expr.node)) return number->value == 0.0;
    if (const auto* unary = std::get_if<UnaryExpr>(&expr.node)) {
        return unary->op == UnaryOp::Neg && unary->operand && is_literal_zero(*unary->operand);
    }
    return false;
}

const BoolExpr* literal_condition(const ExprPtr& condition) {
    return condition ? std::get_if<BoolExpr>(&condition->node) : nullptr;
}

SourcePos stmt_start(const Stmt& stmt) {
    return std::visit([](const auto& node) { return SourcePos{node.line, node.col}; }, stmt.node);
}

bool can_leave_loop(const StmtList& stmts, bool nested);

bool stmt_leaves_loop(const Stmt& stmt, bool nested) {
    return std::visit(
        [nested](const auto& node) -> bool {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, ReturnStmt>) {
                return true;
            } else if constexpr (std::is_same_v<Node, BreakStmt>) {
                return !nested;
            } else if constexpr (std::is_same_v<Node, SeStmt>) {
                return can_leave_loop(node.then_branch, nested) ||
                       can_leave_loop(node.else_branch, nested);
            } else if constexpr (std::is_same_v<Node, EscolhaStmt>) {
                for (const CaseClause& clause : node.cases) {
                    if (can_leave_loop(clause.body, nested)) return true;
                }
                return can_leave_loop(node.default_body, nested);
            } else if constexpr (std::is_same_v<Node, EnquantoStmt> ||
                                 std::is_same_v<Node, ParaStmt> ||
                                 std::is_same_v<Node, RepitaStmt>) {
                return can_leave_loop(node.body, true);
            } else {
                return false;
            }
        },
        stmt.node);
}

bool can_leave_loop(const StmtList& stmts, bool nested) {
    for (const StmtPtr& stmt : stmts) {
        if (stmt && stmt_leaves_loop(*stmt, nested)) return true;
    }
    return false;
}

std::string division_text(BinaryOp op) {
    if (op == BinaryOp::IntDiv) return "divisão por zero no 'div'";
    if (op == BinaryOp::Mod) return "resto ('mod') de uma divisão por zero";
    return "divisão por zero";
}

class FlowChecker {
public:
    explicit FlowChecker(Diagnostics& out) : out_(out) {}

    void block(const StmtList& stmts) {
        bool reported = false;
        for (std::size_t i = 0; i < stmts.size(); ++i) {
            if (!stmts[i]) continue;
            stmt(*stmts[i]);
            if (reported || i + 1 >= stmts.size() || !stmts[i + 1]) continue;
            const bool is_return = std::holds_alternative<ReturnStmt>(stmts[i]->node);
            if (!is_return && !std::holds_alternative<BreakStmt>(stmts[i]->node)) continue;
            reported = true;
            const SourcePos exit = stmt_start(*stmts[i]);
            const SourcePos next = stmt_start(*stmts[i + 1]);
            out_.warning("W009", "este código nunca vai rodar", next.line, next.col,
                         std::string("ele vem logo depois de ") +
                             (is_return ? "um 'retorne'" : "um 'interrompa'") + " (linha " +
                             std::to_string(exit.line) + "), que sai " +
                             (is_return ? "do subprograma" : "do laço") +
                             " antes de chegar aqui. Apague-o ou mova-o para antes.");
        }
    }

private:
    Diagnostics& out_;

    void stmt(const Stmt& stmt) {
        std::visit([this](const auto& node) { visit(node); }, stmt.node);
    }

    void visit(const AssignStmt& node) {
        expr(node.target);
        expr(node.value);
    }
    void visit(const EscrevaStmt& node) {
        for (const ExprPtr& arg : node.args) expr(arg);
    }
    void visit(const LeiaStmt& node) {
        for (const ExprPtr& target : node.targets) expr(target);
    }
    void visit(const SeStmt& node) {
        expr(node.condition);
        if (const BoolExpr* literal = literal_condition(node.condition)) {
            const char* effect = !literal->value ? "o bloco do 'se' nunca roda."
                                 : node.else_branch.empty()
                                     ? "o bloco do 'se' sempre roda."
                                     : "o bloco do 'se' sempre roda e o do 'senao' nunca.";
            constant_condition(*literal, "do 'se'", effect);
        }
        block(node.then_branch);
        block(node.else_branch);
    }
    void visit(const EnquantoStmt& node) {
        expr(node.condition);
        const BoolExpr* literal = literal_condition(node.condition);
        if (literal && !(literal->value && can_leave_loop(node.body, false))) {
            constant_condition(*literal, "do 'enquanto'",
                               literal->value ? kNeverEnds : "o laço nunca roda.");
        }
        block(node.body);
    }
    void visit(const ParaStmt& node) {
        expr(node.from);
        expr(node.to);
        expr(node.step);
        block(node.body);
    }
    void visit(const RepitaStmt& node) {
        block(node.body);
        expr(node.condition);
        const BoolExpr* literal = literal_condition(node.condition);
        if (literal && (literal->value || !can_leave_loop(node.body, false))) {
            constant_condition(*literal, "do 'ate'",
                               literal->value ? "o laço roda uma vez só." : kNeverEnds);
        }
    }
    void visit(const EscolhaStmt& node) {
        expr(node.selector);
        for (const CaseClause& clause : node.cases) {
            for (const CaseLabel& label : clause.labels) {
                expr(label.value);
                expr(label.range_end);
            }
            block(clause.body);
        }
        block(node.default_body);
    }
    void visit(const CallStmt& node) {
        for (const ExprPtr& arg : node.args) expr(arg);
    }
    void visit(const ReturnStmt& node) { expr(node.value); }
    void visit(const BreakStmt&) {}

    static constexpr const char* kNeverEnds =
        "o laço nunca termina. Use 'interrompa' para sair dele ou troque a condição.";

    void constant_condition(const BoolExpr& literal, const char* place, const char* effect) {
        out_.warning("W008",
                     std::string("a condição ") + place + " é sempre " +
                         (literal.value ? "verdadeira" : "falsa"),
                     literal.line, literal.col, effect);
    }

    void expr(const ExprPtr& expr_ptr) {
        if (!expr_ptr) return;
        std::visit(
            [this](const auto& node) {
                using Node = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<Node, BinaryExpr>) {
                    binary(node);
                } else if constexpr (std::is_same_v<Node, UnaryExpr>) {
                    expr(node.operand);
                } else if constexpr (std::is_same_v<Node, IndexExpr>) {
                    for (const ExprPtr& index : node.indices) expr(index);
                } else if constexpr (std::is_same_v<Node, CallExpr>) {
                    for (const ExprPtr& arg : node.args) expr(arg);
                }
            },
            expr_ptr->node);
    }

    void binary(const BinaryExpr& node) {
        expr(node.left);
        expr(node.right);
        const bool divides =
            node.op == BinaryOp::Div || node.op == BinaryOp::IntDiv || node.op == BinaryOp::Mod;
        if (!divides || !node.right || !is_literal_zero(*node.right)) return;
        const SourcePos pos = expr_start(*node.right);
        out_.warning("W007", division_text(node.op), pos.line, pos.col,
                     "quando essa conta rodar, o programa para com o erro R001. Troque o 0 pelo "
                     "valor certo.");
    }
};

} // namespace

void check_flow_warnings(const Program& program, Diagnostics& out) {
    FlowChecker checker(out);
    for (const ProcedureDecl& proc : program.procedures) checker.block(proc.body);
    checker.block(program.body);
}

} // namespace spike::warnings_detail
