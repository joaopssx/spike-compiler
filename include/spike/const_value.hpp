#ifndef SPIKE_CONST_VALUE_HPP
#define SPIKE_CONST_VALUE_HPP

#include "spike/ast.hpp"

#include <string>
#include <unordered_map>

namespace spike {

struct ConstValue {
    TypeKind type = TypeKind::Invalid;
    long long integer = 0;
    double real = 0.0;
    std::string text;
    bool logical = false;
};

using ConstantMap = std::unordered_map<std::string, ConstValue>;

struct ConstEvaluation {
    bool ok = false;
    ConstValue value;
    std::string problem;
    SourcePos pos;
};

ConstEvaluation evaluate_constant(const Expr& expr, const ConstantMap& constants);

} // namespace spike

#endif // SPIKE_CONST_VALUE_HPP
