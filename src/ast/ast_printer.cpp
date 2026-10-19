#include "spike/ast_printer.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <variant>

namespace spike {

namespace {

std::string position(int line, int col) {
    return " (" + std::to_string(line) + ":" + std::to_string(col) + ")";
}

std::string number_text(const NumberExpr& node) {
    char buffer[40];
    if (node.is_integer) {
        std::snprintf(buffer, sizeof buffer, "%lld", static_cast<long long>(node.value));
        return buffer;
    }
    for (int precision = 1; precision <= 17; ++precision) {
        std::snprintf(buffer, sizeof buffer, "%.*g", precision, node.value);
        if (std::strtod(buffer, nullptr) == node.value) break;
    }
    std::string text = buffer;
    if (text.find_first_of(".e") == std::string::npos) text += ".0";
    return text;
}

std::string bound_text(const ArrayBound& bound) {
    const std::string sign = bound.negative && !bound.constant.empty() ? "-" : "";
    return bound.constant.empty() ? std::to_string(bound.value) : sign + bound.constant;
}

std::string array_text(const VarDecl& decl) {
    if (decl.dims.empty()) return "";
    std::string text = "vetor[";
    for (std::size_t i = 0; i < decl.dims.size(); ++i) {
        if (i > 0) text += ", ";
        text += bound_text(decl.dims[i].low) + ".." + bound_text(decl.dims[i].high);
    }
    return text + "] de ";
}

std::string quoted_text(const std::string& value) {
    std::string out = "\"";
    for (char c : value) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\t') {
            out += "\\t";
        } else {
            out += c;
        }
    }
    return out + "\"";
}

class Printer {
public:
    std::string out;

    void line(int depth, const std::string& text) {
        out.append(static_cast<std::size_t>(depth) * 2, ' ');
        out += text;
        out += '\n';
    }

    void expr(const ExprPtr& value, int depth) {
        if (!value) {
            line(depth, "<vazio>");
            return;
        }
        std::visit(
            [this, depth](const auto& node) {
                using Node = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<Node, NumberExpr>) {
                    line(depth, "Number " + number_text(node));
                } else if constexpr (std::is_same_v<Node, StringExpr>) {
                    line(depth, "String " + quoted_text(node.value));
                } else if constexpr (std::is_same_v<Node, BoolExpr>) {
                    line(depth, std::string("Bool ") + (node.value ? "verdadeiro" : "falso"));
                } else if constexpr (std::is_same_v<Node, VarExpr>) {
                    line(depth, "Var " + node.name);
                } else if constexpr (std::is_same_v<Node, IndexExpr>) {
                    line(depth, "Index " + node.name + position(node.line, node.col));
                    for (const ExprPtr& index : node.indices) expr(index, depth + 1);
                } else if constexpr (std::is_same_v<Node, CallExpr>) {
                    line(depth, "Call " + node.name + position(node.line, node.col));
                    for (const ExprPtr& arg : node.args) expr(arg, depth + 1);
                } else if constexpr (std::is_same_v<Node, BinaryExpr>) {
                    line(depth, std::string("Binary ") + to_string(node.op) +
                                    position(node.line, node.col));
                    expr(node.left, depth + 1);
                    expr(node.right, depth + 1);
                } else {
                    line(depth, std::string("Unary ") + to_string(node.op) +
                                    position(node.line, node.col));
                    expr(node.operand, depth + 1);
                }
            },
            value->node);
    }

    void var(const VarDecl& decl, int depth) {
        line(depth, "Var " + decl.name + " : " + array_text(decl) + type_name(decl.type) +
                        position(decl.line, decl.col));
    }

    void labeled_block(const char* label, const StmtList& stmts, int depth) {
        line(depth, label);
        block(stmts, depth + 1);
    }

    void labeled_expr(const char* label, const ExprPtr& value, int depth) {
        line(depth, label);
        expr(value, depth + 1);
    }

    void block(const StmtList& stmts, int depth) {
        for (const StmtPtr& stmt : stmts) {
            if (stmt) statement(*stmt, depth);
        }
    }

    void statement(const Stmt& stmt, int depth) {
        std::visit([this, depth](const auto& node) { visit_stmt(node, depth); }, stmt.node);
    }

    void visit_stmt(const AssignStmt& node, int depth) {
        const VarExpr* var = target_variable(node.target);
        line(depth, "Assign " + (var ? var->name : std::string("?")) + position(node.line, node.col));
        expr(node.value, depth + 1);
    }

    void visit_stmt(const EscrevaStmt& node, int depth) {
        line(depth, (node.newline ? "Escreval" : "Escreva") + position(node.line, node.col));
        for (const ExprPtr& arg : node.args) expr(arg, depth + 1);
    }

    void visit_stmt(const LeiaStmt& node, int depth) {
        std::string names;
        for (const ExprPtr& target : node.targets) {
            const VarExpr* var = target_variable(target);
            names += (names.empty() ? "" : ", ") + (var ? var->name : std::string("?"));
        }
        line(depth, "Leia " + names + position(node.line, node.col));
    }

    void visit_stmt(const SeStmt& node, int depth) {
        line(depth, "Se" + position(node.line, node.col));
        expr(node.condition, depth + 1);
        labeled_block("entao", node.then_branch, depth + 1);
        if (!node.else_branch.empty()) labeled_block("senao", node.else_branch, depth + 1);
    }

    void visit_stmt(const EnquantoStmt& node, int depth) {
        line(depth, "Enquanto" + position(node.line, node.col));
        expr(node.condition, depth + 1);
        labeled_block("faca", node.body, depth + 1);
    }

    void visit_stmt(const ParaStmt& node, int depth) {
        line(depth, "Para " + node.var_name + position(node.line, node.col));
        labeled_expr("de", node.from, depth + 1);
        labeled_expr("ate", node.to, depth + 1);
        if (node.step) labeled_expr("passo", node.step, depth + 1);
        labeled_block("faca", node.body, depth + 1);
    }

    void visit_stmt(const EscolhaStmt& node, int depth) {
        line(depth, "Escolha" + position(node.line, node.col));
        expr(node.selector, depth + 1);
        for (const CaseClause& clause : node.cases) {
            line(depth + 1, "caso" + position(clause.line, clause.col));
            for (const CaseLabel& label : clause.labels) {
                if (label.range_end) {
                    line(depth + 2, "intervalo");
                    expr(label.value, depth + 3);
                    expr(label.range_end, depth + 3);
                } else {
                    expr(label.value, depth + 2);
                }
            }
            labeled_block("corpo", clause.body, depth + 2);
        }
        if (node.has_default) labeled_block("outrocaso", node.default_body, depth + 1);
    }

    void visit_stmt(const BreakStmt& node, int depth) {
        line(depth, "Interrompa" + position(node.line, node.col));
    }

    void visit_stmt(const CallStmt& node, int depth) {
        line(depth, "Call " + node.name + position(node.line, node.col));
        for (const ExprPtr& arg : node.args) expr(arg, depth + 1);
    }

    void visit_stmt(const ReturnStmt& node, int depth) {
        line(depth, "Retorne" + position(node.line, node.col));
        if (node.value) expr(node.value, depth + 1);
    }

    void visit_stmt(const RepitaStmt& node, int depth) {
        line(depth, "Repita" + position(node.line, node.col));
        labeled_block("corpo", node.body, depth + 1);
        labeled_expr("ate", node.condition, depth + 1);
    }
};

std::size_t count_in(const Stmt& stmt) {
    return 1 + std::visit(
                   [](const auto& node) -> std::size_t {
                       using Node = std::decay_t<decltype(node)>;
                       if constexpr (std::is_same_v<Node, SeStmt>) {
                           return count_statements(node.then_branch) +
                                  count_statements(node.else_branch);
                       } else if constexpr (std::is_same_v<Node, EscolhaStmt>) {
                           std::size_t total = count_statements(node.default_body);
                           for (const CaseClause& clause : node.cases) {
                               total += count_statements(clause.body);
                           }
                           return total;
                       } else if constexpr (std::is_same_v<Node, EnquantoStmt> ||
                                            std::is_same_v<Node, ParaStmt> ||
                                            std::is_same_v<Node, RepitaStmt>) {
                           return count_statements(node.body);
                       } else {
                           return 0;
                       }
                   },
                   stmt.node);
}

} // namespace

std::string print_ast(const Program& program) {
    Printer printer;
    printer.line(0, "Program " + quoted_text(program.name) + position(program.line, 1));
    for (const ConstDecl& decl : program.consts) {
        printer.line(1, "Const " + decl.name + position(decl.line, decl.col));
        printer.expr(decl.value, 2);
    }
    for (const VarDecl& decl : program.globals) printer.var(decl, 1);
    for (const ProcedureDecl& proc : program.procedures) {
        printer.line(1, proc.is_function()
                            ? "Function " + proc.name + " : " + type_name(proc.return_type) +
                                  position(proc.line, proc.col)
                            : "Procedure " + proc.name + position(proc.line, proc.col));
        for (const Param& param : proc.params) {
            printer.line(2, "Param " + param.name + " : " + type_name(param.type) +
                                position(param.line, param.col));
        }
        for (const VarDecl& decl : proc.locals) printer.var(decl, 2);
        printer.block(proc.body, 2);
    }
    printer.block(program.body, 1);
    return printer.out;
}

std::size_t count_statements(const StmtList& stmts) {
    std::size_t total = 0;
    for (const StmtPtr& stmt : stmts) {
        if (stmt) total += count_in(*stmt);
    }
    return total;
}

std::size_t count_statements(const Program& program) {
    std::size_t total = count_statements(program.body);
    for (const ProcedureDecl& proc : program.procedures) total += count_statements(proc.body);
    return total;
}

} // namespace spike
