#include "spike/ast_enums.hpp"

namespace spike
{

    const char *to_string(BinaryOp op)
    {
        switch (op)
        {
        case BinaryOp::Add:
            return "+";
        case BinaryOp::Sub:
            return "-";
        case BinaryOp::Mul:
            return "*";
        case BinaryOp::Div:
            return "/";
        case BinaryOp::IntDiv:
            return "div";
        case BinaryOp::Mod:
            return "mod";
        case BinaryOp::Pow:
            return "^";
        case BinaryOp::Eq:
            return "=";
        case BinaryOp::Ne:
            return "<>";
        case BinaryOp::Lt:
            return "<";
        case BinaryOp::Gt:
            return ">";
        case BinaryOp::Le:
            return "<=";
        case BinaryOp::Ge:
            return ">=";
        case BinaryOp::And:
            return "e";
        case BinaryOp::Or:
            return "ou";
        }
        return "?";
    }

    const char *to_string(UnaryOp op)
    {
        switch (op)
        {
        case UnaryOp::Neg:
            return "-";
        case UnaryOp::Not:
            return "nao";
        }
        return "?";
    }

    const char *type_name(TypeKind type)
    {
        switch (type)
        {
        case TypeKind::Integer:
            return "inteiro";
        case TypeKind::Real:
            return "real";
        case TypeKind::String:
            return "caractere";
        case TypeKind::Bool:
            return "logico";
        case TypeKind::None:
        case TypeKind::Invalid:
            return "?";
        }
        return "?";
    }

} // namespace spike
