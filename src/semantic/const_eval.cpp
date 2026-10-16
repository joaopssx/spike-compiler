#include "spike/const_value.hpp"
#include "spike/symbol_table.hpp"

#include <climits>
#include <cmath>
#include <string>
#include <type_traits>
#include <variant>

namespace spike {

namespace {

struct Failure {
    std::string problem;
    SourcePos pos;
};

double as_real(const ConstValue& v) {
    return v.type == TypeKind::Integer ? static_cast<double>(v.integer) : v.real;
}

ConstValue make_integer(long long value) {
    ConstValue v;
    v.type = TypeKind::Integer;
    v.integer = value;
    return v;
}

ConstValue make_real(double value) {
    ConstValue v;
    v.type = TypeKind::Real;
    v.real = value;
    return v;
}

ConstValue make_bool(bool value) {
    ConstValue v;
    v.type = TypeKind::Bool;
    v.logical = value;
    return v;
}

long long wrap(unsigned long long value) {
    return static_cast<long long>(value);
}

class Evaluator {
public:
    explicit Evaluator(const ConstantMap& constants) : constants_(constants) {}

    std::variant<ConstValue, Failure> eval(const Expr& expr) {
        return std::visit([this](const auto& node) { return visit(node); }, expr.node);
    }

private:
    const ConstantMap& constants_;

    std::variant<ConstValue, Failure> visit(const NumberExpr& node) {
        if (node.is_integer) return make_integer(static_cast<long long>(node.value));
        return make_real(node.value);
    }

    std::variant<ConstValue, Failure> visit(const StringExpr& node) {
        ConstValue v;
        v.type = TypeKind::String;
        v.text = node.value;
        return v;
    }

    std::variant<ConstValue, Failure> visit(const BoolExpr& node) { return make_bool(node.value); }

    std::variant<ConstValue, Failure> visit(const VarExpr& node) {
        const auto it = constants_.find(symbol_key(node.name));
        if (it == constants_.end()) {
            return Failure{"'" + node.name + "' não tem valor fixo", {node.line, node.col}};
        }
        return it->second;
    }

    std::variant<ConstValue, Failure> visit(const UnaryExpr& node) {
        auto operand = eval(*node.operand);
        if (std::holds_alternative<Failure>(operand)) return operand;
        const ConstValue v = std::get<ConstValue>(operand);
        if (node.op == UnaryOp::Not) return make_bool(!v.logical);
        if (v.type == TypeKind::Integer) {
            return make_integer(wrap(0ULL - static_cast<unsigned long long>(v.integer)));
        }
        return make_real(-v.real);
    }

    std::variant<ConstValue, Failure> visit(const BinaryExpr& node) {
        auto left = eval(*node.left);
        if (std::holds_alternative<Failure>(left)) return left;
        auto right = eval(*node.right);
        if (std::holds_alternative<Failure>(right)) return right;
        return combine(node, std::get<ConstValue>(left), std::get<ConstValue>(right));
    }

    static std::variant<ConstValue, Failure> combine(const BinaryExpr& node, const ConstValue& a,
                                                     const ConstValue& b) {
        const SourcePos pos{node.line, node.col};
        const bool ints = a.type == TypeKind::Integer && b.type == TypeKind::Integer;
        const auto ua = static_cast<unsigned long long>(a.integer);
        const auto ub = static_cast<unsigned long long>(b.integer);
        switch (node.op) {
            case BinaryOp::Add:
                if (a.type == TypeKind::String) {
                    ConstValue v = a;
                    v.text += b.text;
                    return v;
                }
                return ints ? make_integer(wrap(ua + ub)) : make_real(as_real(a) + as_real(b));
            case BinaryOp::Sub:
                return ints ? make_integer(wrap(ua - ub)) : make_real(as_real(a) - as_real(b));
            case BinaryOp::Mul:
                return ints ? make_integer(wrap(ua * ub)) : make_real(as_real(a) * as_real(b));
            case BinaryOp::Div:
                if (as_real(b) == 0.0) return Failure{"divisão por zero", pos};
                return make_real(as_real(a) / as_real(b));
            case BinaryOp::IntDiv:
            case BinaryOp::Mod:
                if (b.integer == 0) return Failure{"divisão por zero", pos};
                if (b.integer == -1) {
                    return make_integer(node.op == BinaryOp::Mod ? 0 : wrap(0ULL - ua));
                }
                return make_integer(node.op == BinaryOp::Mod ? a.integer % b.integer
                                                             : a.integer / b.integer);
            case BinaryOp::Pow: {
                const double result = std::pow(as_real(a), as_real(b));
                if (std::isnan(result) || std::isinf(result)) {
                    return Failure{"resultado inválido na potência", pos};
                }
                return make_real(result);
            }
            case BinaryOp::And: return make_bool(a.logical && b.logical);
            case BinaryOp::Or: return make_bool(a.logical || b.logical);
            default: return make_bool(compare(node.op, a, b));
        }
    }

    static bool compare(BinaryOp op, const ConstValue& a, const ConstValue& b) {
        int order = 0;
        if (a.type == TypeKind::String) {
            order = a.text.compare(b.text);
        } else if (a.type == TypeKind::Bool) {
            order = static_cast<int>(a.logical) - static_cast<int>(b.logical);
        } else if (a.type == TypeKind::Integer && b.type == TypeKind::Integer) {
            order = (a.integer > b.integer) - (a.integer < b.integer);
        } else {
            order = (as_real(a) > as_real(b)) - (as_real(a) < as_real(b));
        }
        switch (op) {
            case BinaryOp::Eq: return order == 0;
            case BinaryOp::Ne: return order != 0;
            case BinaryOp::Lt: return order < 0;
            case BinaryOp::Gt: return order > 0;
            case BinaryOp::Le: return order <= 0;
            default: return order >= 0;
        }
    }
};

} // namespace

ConstEvaluation evaluate_constant(const Expr& expr, const ConstantMap& constants) {
    Evaluator evaluator(constants);
    auto result = evaluator.eval(expr);
    ConstEvaluation out;
    if (const auto* failure = std::get_if<Failure>(&result)) {
        out.problem = failure->problem;
        out.pos = failure->pos;
        return out;
    }
    out.ok = true;
    out.value = std::get<ConstValue>(result);
    return out;
}

} // namespace spike
