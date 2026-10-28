#include "codegen/codegen_internal.hpp"
#include "spike/symbol_table.hpp"

#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace spike::codegen_detail {

namespace {

struct Visitor {
    std::function<void(const Stmt&)> stmt;
    std::function<void(const std::string&, const std::vector<ExprPtr>&)> call;
};

void walk_block(const StmtList& stmts, const Visitor& visitor);

void walk_expr(const ExprPtr& value, const Visitor& visitor) {
    if (!value) return;
    std::visit(
        [&visitor](const auto& node) {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, BinaryExpr>) {
                walk_expr(node.left, visitor);
                walk_expr(node.right, visitor);
            } else if constexpr (std::is_same_v<Node, UnaryExpr>) {
                walk_expr(node.operand, visitor);
            } else if constexpr (std::is_same_v<Node, IndexExpr>) {
                for (const ExprPtr& index : node.indices) walk_expr(index, visitor);
            } else if constexpr (std::is_same_v<Node, CallExpr>) {
                visitor.call(node.name, node.args);
                for (const ExprPtr& arg : node.args) walk_expr(arg, visitor);
            }
        },
        value->node);
}

void walk_stmt(const Stmt& stmt, const Visitor& visitor) {
    visitor.stmt(stmt);
    std::visit(
        [&visitor](const auto& node) {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, AssignStmt>) {
                walk_expr(node.target, visitor);
                walk_expr(node.value, visitor);
            } else if constexpr (std::is_same_v<Node, EscrevaStmt> ||
                                 std::is_same_v<Node, CallStmt>) {
                if constexpr (std::is_same_v<Node, CallStmt>) visitor.call(node.name, node.args);
                for (const ExprPtr& arg : node.args) walk_expr(arg, visitor);
            } else if constexpr (std::is_same_v<Node, LeiaStmt>) {
                for (const ExprPtr& target : node.targets) walk_expr(target, visitor);
            } else if constexpr (std::is_same_v<Node, SeStmt>) {
                walk_expr(node.condition, visitor);
                walk_block(node.then_branch, visitor);
                walk_block(node.else_branch, visitor);
            } else if constexpr (std::is_same_v<Node, EnquantoStmt> ||
                                 std::is_same_v<Node, RepitaStmt>) {
                walk_expr(node.condition, visitor);
                walk_block(node.body, visitor);
            } else if constexpr (std::is_same_v<Node, ParaStmt>) {
                walk_expr(node.from, visitor);
                walk_expr(node.to, visitor);
                walk_expr(node.step, visitor);
                walk_block(node.body, visitor);
            } else if constexpr (std::is_same_v<Node, EscolhaStmt>) {
                walk_expr(node.selector, visitor);
                for (const CaseClause& clause : node.cases) walk_block(clause.body, visitor);
                walk_block(node.default_body, visitor);
            } else if constexpr (std::is_same_v<Node, ReturnStmt>) {
                walk_expr(node.value, visitor);
            }
        },
        stmt.node);
}

void walk_block(const StmtList& stmts, const Visitor& visitor) {
    for (const StmtPtr& stmt : stmts) {
        if (stmt) walk_stmt(*stmt, visitor);
    }
}

const std::string* target_name(const ExprPtr& target) {
    if (const VarExpr* var = target_variable(target)) return &var->name;
    if (const auto* index = target ? std::get_if<IndexExpr>(&target->node) : nullptr) {
        return &index->name;
    }
    return nullptr;
}

std::unordered_set<std::string> own_names(const ProcedureDecl& proc) {
    std::unordered_set<std::string> names;
    for (const Param& param : proc.params) {
        if (!param.by_ref) names.insert(symbol_key(param.name));
    }
    for (const VarDecl& decl : proc.locals) names.insert(symbol_key(decl.name));
    return names;
}

} // namespace

Access operator|(Access a, Access b) {
    return Access{a.reads || b.reads, a.changes || b.changes};
}

void Generator::collect_effects() {
    bool grew = true;
    while (grew) {
        grew = false;
        for (const ProcedureDecl& proc : program_.procedures) {
            const std::string key = symbol_key(proc.name);
            if (with_effects_.count(key) == 0 && has_effects(proc)) {
                with_effects_.insert(key);
                grew = true;
            }
        }
    }
}

// A subprogram has effects when calling it can change something the caller sees: a global,
// a parameter by reference, the input or the output.
bool Generator::has_effects(const ProcedureDecl& proc) const {
    const std::unordered_set<std::string> own = own_names(proc);
    const auto outside = [&own](const std::string* name) {
        return name && own.count(symbol_key(*name)) == 0;
    };
    bool found = false;
    Visitor visitor;
    visitor.stmt = [&](const Stmt& stmt) {
        if (std::holds_alternative<EscrevaStmt>(stmt.node) ||
            std::holds_alternative<LeiaStmt>(stmt.node)) {
            found = true;
        } else if (const auto* assign = std::get_if<AssignStmt>(&stmt.node)) {
            if (outside(target_name(assign->target))) found = true;
        } else if (const auto* para = std::get_if<ParaStmt>(&stmt.node)) {
            if (outside(&para->var_name)) found = true;
        }
    };
    visitor.call = [&](const std::string& name, const std::vector<ExprPtr>&) {
        if (with_effects_.count(symbol_key(name)) != 0) found = true;
    };
    walk_block(proc.body, visitor);
    return found;
}

void Generator::enter_scope(const ProcedureDecl* proc) {
    locals_.clear();
    locals_by_ref_.clear();
    if (!proc) return;
    locals_ = own_names(*proc);
    Visitor visitor;
    visitor.stmt = [](const Stmt&) {};
    visitor.call = [this](const std::string& name, const std::vector<ExprPtr>& args) {
        const auto it = procedures_.find(symbol_key(name));
        if (it == procedures_.end()) return;
        const std::vector<Param>& params = it->second->params;
        for (std::size_t i = 0; i < args.size() && i < params.size(); ++i) {
            const std::string* passed = params[i].by_ref ? target_name(args[i]) : nullptr;
            if (passed) locals_by_ref_.insert(symbol_key(*passed));
        }
    };
    walk_block(proc->body, visitor);
}

bool Generator::may_change(const std::string& name) const {
    const std::string key = symbol_key(name);
    if (locals_.count(key) != 0) return locals_by_ref_.count(key) != 0;
    if (ref_params_.count(key) != 0) return true;
    return constants_.count(key) == 0;
}

Access Generator::access(const ExprPtr& value) const {
    if (!value) return Access{};
    return std::visit(
        [this](const auto& node) -> Access {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, VarExpr>) {
                return Access{may_change(node.name), false};
            } else if constexpr (std::is_same_v<Node, IndexExpr>) {
                Access result{may_change(node.name), false};
                for (const ExprPtr& index : node.indices) result = result | access(index);
                return result;
            } else if constexpr (std::is_same_v<Node, CallExpr>) {
                const std::string key = symbol_key(node.name);
                Access result{true, with_effects_.count(key) != 0 || key == "aleatorio"};
                for (const ExprPtr& arg : node.args) result = result | access(arg);
                return result;
            } else if constexpr (std::is_same_v<Node, BinaryExpr>) {
                return access(node.left) | access(node.right);
            } else if constexpr (std::is_same_v<Node, UnaryExpr>) {
                return access(node.operand);
            } else {
                return Access{};
            }
        },
        value->node);
}

Access Generator::ref_access(const ExprPtr& arg) const {
    Access result;
    if (const auto* index = arg ? std::get_if<IndexExpr>(&arg->node) : nullptr) {
        for (const ExprPtr& position : index->indices) result = result | access(position);
    }
    return result;
}

Access Generator::arg_access(const ProcedureDecl& proc, const std::vector<ExprPtr>& args,
                             std::size_t from) const {
    Access result;
    for (std::size_t i = from; i < args.size() && i < proc.params.size(); ++i) {
        result = result | (proc.params[i].by_ref ? ref_access(args[i]) : access(args[i]));
    }
    return result;
}

bool Generator::must_come_first(const Access& part, const Access& after) const {
    return (part.changes && after.reads) || (after.changes && part.reads);
}

std::string Generator::temporary(const std::string& type, const std::string& value) {
    const std::string name = "spike_tmp_" + std::to_string(next_loop_id());
    pending_.push_back(type + " " + name + " = " + value + ";");
    return name;
}

std::string Generator::ordered_reference(const ExprPtr& arg, const Access& after) {
    const Access outer = later_;
    const Access later = outer | after;
    const bool first = must_come_first(ref_access(arg), later);
    later_ = first ? Access{} : later;
    const std::string text = reference(arg);
    later_ = outer;
    if (!first || failed() || !arg) return text;
    const char* type = c_type(type_of(*arg));
    return temporary(std::string(type ? type : "int") + "*", text);
}

std::string Generator::logical(const BinaryExpr& node) {
    const std::string left = expr(node.left);
    std::vector<std::string> before = take_pending();
    const int first_id = loop_count_;
    std::string right = expr(node.right);
    std::vector<std::string> inside = take_pending();
    pending_ = std::move(before);
    if (failed()) return "";
    const bool is_and = node.op == BinaryOp::And;
    if (inside.empty()) return "(" + left + (is_and ? " && " : " || ") + right + ")";
    // Generated again so the temporary of the whole test is numbered before the ones inside.
    loop_count_ = first_id;
    const std::string name = temporary("spike_bool", left);
    before = take_pending();
    right = expr(node.right);
    inside = take_pending();
    pending_ = std::move(before);
    pending_.push_back(std::string("if (") + (is_and ? "" : "!") + name + ") {");
    for (const std::string& line : inside) pending_.push_back("    " + line);
    pending_.push_back("    " + name + " = " + right + ";");
    pending_.push_back("}");
    return name;
}

std::vector<std::string> Generator::take_pending() {
    std::vector<std::string> lines = std::move(pending_);
    pending_.clear();
    return lines;
}

void Generator::emit_lines(const std::vector<std::string>& lines) {
    for (const std::string& line : lines) out_.line(line);
}

void Generator::emit_line(const std::string& text) {
    if (pending_.empty()) {
        out_.line(text);
        return;
    }
    out_.line("{");
    out_.indent();
    emit_lines(take_pending());
    out_.line(text);
    out_.dedent();
    out_.line("}");
}

} // namespace spike::codegen_detail
