#ifndef SPIKE_AST_ENUMS_HPP
#define SPIKE_AST_ENUMS_HPP

namespace spike {

enum class BinaryOp { Add, Sub, Mul, Div, Eq, Ne, Lt, Gt, Le, Ge, And, Or };

enum class UnaryOp { Neg, Not };

enum class TypeKind { None, Invalid, Integer, Real, String, Bool };

const char* to_string(BinaryOp op);
const char* to_string(UnaryOp op);
const char* type_name(TypeKind type);

} // namespace spike

#endif // SPIKE_AST_ENUMS_HPP
