#include "ast.hpp"

#include <sstream>
#include <type_traits>
#include <variant>

#include "token.hpp"

namespace {

struct ExpressionPrinter {
    std::string operator()(const IdentifierExpr& expression) const {
        return expression.name;
    }

    std::string operator()(const IntegerLiteralExpr& expression) const {
        return expression.value;
    }

    std::string operator()(const RealLiteralExpr& expression) const {
        return expression.value;
    }

    std::string operator()(const StringLiteralExpr& expression) const {
        return '"' + escape_lexeme(expression.value) + '"';
    }
};

}  // namespace

std::string expression_to_string(const Expression& expression) {
    return std::visit(ExpressionPrinter{}, expression);
}

std::string ast_to_string(const Program& program) {
    std::ostringstream output;
    output << "Program\n";

    for (const VarDecl& variable : program.variables) {
        output << "  VarDecl " << variable.name << " : " << variable.type_name << "\n";
    }

    for (const Statement& statement : program.statements) {
        std::visit(
            [&output](const auto& item) {
                using T = std::decay_t<decltype(item)>;

                if constexpr (std::is_same_v<T, AssignStmt>) {
                    output << "  Assign " << item.target << " <- "
                           << expression_to_string(item.value) << "\n";
                } else if constexpr (std::is_same_v<T, WriteStmt>) {
                    output << "  Write " << expression_to_string(item.value) << "\n";
                }
            },
            statement);
    }

    std::string text = output.str();
    if (!text.empty() && text.back() == '\n') {
        text.pop_back();
    }

    return text;
}
