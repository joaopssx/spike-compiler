#include "codegen.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace portugol {

namespace {

std::string cOperator(const std::string& op) {
    if (op == "=") {
        return "==";
    }
    if (op == "<>") {
        return "!=";
    }
    return op;
}

std::string formatReal(double value) {
    std::ostringstream out;
    out << std::setprecision(15) << value;
    return out.str();
}

} // namespace

std::string CodeGenerator::generate(const Program& program) const {
    std::string out;
    out += "#include <stdio.h>\n\n";
    out += "int main(void) {\n";

    for (const auto& declaration : program.declarations) {
        const auto* varDecl = dynamic_cast<const VarDeclStmt*>(declaration.get());
        if (varDecl == nullptr) {
            throw std::runtime_error("declaracao AST invalida no gerador de codigo");
        }

        for (const auto& name : varDecl->names) {
            out += emitIndent(1) + cType(varDecl->type) + " " + name + ";\n";
        }
    }

    if (!program.declarations.empty() && !program.statements.empty()) {
        out += "\n";
    }

    for (const auto& stmt : program.statements) {
        emitStmt(*stmt, 1, out);
    }

    if (!program.statements.empty()) {
        out += "\n";
    }

    out += emitIndent(1) + "return 0;\n";
    out += "}\n";
    return out;
}

void CodeGenerator::emitStmt(const Stmt& stmt, std::size_t indentLevel, std::string& out) const {
    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        out += emitIndent(indentLevel) + assign->name + " = " + emitExpr(*assign->value) + ";\n";
        return;
    }

    if (const auto* write = dynamic_cast<const WriteStmt*>(&stmt)) {
        out += emitIndent(indentLevel) + "printf(\"" + printfFormat(write->expr->inferredType) + "\", " + emitExpr(*write->expr) + ");\n";
        return;
    }

    if (const auto* read = dynamic_cast<const ReadStmt*>(&stmt)) {
        out += emitIndent(indentLevel) + "scanf(\"" + scanfFormat(read->targetType) + "\", &" + read->name + ");\n";
        return;
    }

    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        out += emitIndent(indentLevel) + "if (" + emitExpr(*ifStmt->condition) + ") {\n";
        for (const auto& child : ifStmt->thenBranch) {
            emitStmt(*child, indentLevel + 1, out);
        }
        out += emitIndent(indentLevel) + "}";
        if (ifStmt->elseBranch.empty()) {
            out += "\n";
            return;
        }
        out += " else {\n";
        for (const auto& child : ifStmt->elseBranch) {
            emitStmt(*child, indentLevel + 1, out);
        }
        out += emitIndent(indentLevel) + "}\n";
        return;
    }

    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        out += emitIndent(indentLevel) + "while (" + emitExpr(*whileStmt->condition) + ") {\n";
        for (const auto& child : whileStmt->body) {
            emitStmt(*child, indentLevel + 1, out);
        }
        out += emitIndent(indentLevel) + "}\n";
        return;
    }

    throw std::runtime_error("comando AST nao suportado no gerador de codigo");
}

std::string CodeGenerator::emitExpr(const Expr& expr) const {
    if (const auto* intExpr = dynamic_cast<const IntExpr*>(&expr)) {
        return std::to_string(intExpr->value);
    }

    if (const auto* realExpr = dynamic_cast<const RealExpr*>(&expr)) {
        return formatReal(realExpr->value);
    }

    if (const auto* variableExpr = dynamic_cast<const VariableExpr*>(&expr)) {
        return variableExpr->name;
    }

    if (const auto* unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) {
        return "(" + unaryExpr->op + emitExpr(*unaryExpr->operand) + ")";
    }

    if (const auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
        const std::string left = emitExpr(*binaryExpr->left);
        const std::string right = emitExpr(*binaryExpr->right);

        if (binaryExpr->op == "/" && binaryExpr->left->inferredType == ValueType::Inteiro && binaryExpr->right->inferredType == ValueType::Inteiro) {
            return "((double) " + left + " / (double) " + right + ")";
        }

        return "(" + left + " " + cOperator(binaryExpr->op) + " " + right + ")";
    }

    throw std::runtime_error("expressao AST nao suportada no gerador de codigo");
}

std::string CodeGenerator::emitIndent(std::size_t indentLevel) const {
    return std::string(indentLevel * 4, ' ');
}

std::string CodeGenerator::cType(ValueType type) const {
    switch (type) {
    case ValueType::Inteiro:
        return "int";
    case ValueType::Real:
        return "double";
    case ValueType::Logico:
        return "int";
    case ValueType::Caractere:
        return "char";
    case ValueType::Unknown:
        break;
    }

    throw std::runtime_error("tipo invalido no gerador de codigo");
}

std::string CodeGenerator::printfFormat(ValueType type) const {
    switch (type) {
    case ValueType::Inteiro:
        return "%d\\n";
    case ValueType::Real:
        return "%g\\n";
    case ValueType::Logico:
        return "%d\\n";
    case ValueType::Caractere:
        return "%c\\n";
    case ValueType::Unknown:
        break;
    }

    throw std::runtime_error("tipo invalido para printf");
}

std::string CodeGenerator::scanfFormat(ValueType type) const {
    switch (type) {
    case ValueType::Inteiro:
        return "%d";
    case ValueType::Real:
        return "%lf";
    case ValueType::Logico:
        return "%d";
    case ValueType::Caractere:
        return " %c";
    case ValueType::Unknown:
        return "%d";
    }

    return "%d";
}

} // namespace portugol
