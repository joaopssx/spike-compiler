#include "ir_builder.hpp"

#include <sstream>

namespace portugol {

namespace {

std::string opcodeForBinary(std::string_view op) {
    if (op == "+") {
        return "ADD";
    }
    if (op == "-") {
        return "SUB";
    }
    if (op == "*") {
        return "MUL";
    }
    if (op == "/") {
        return "DIV";
    }
    if (op == "=") {
        return "CMP_EQ";
    }
    if (op == "<>") {
        return "CMP_NE";
    }
    if (op == "<") {
        return "CMP_LT";
    }
    if (op == "<=") {
        return "CMP_LE";
    }
    if (op == ">") {
        return "CMP_GT";
    }
    if (op == ">=") {
        return "CMP_GE";
    }
    return "OP";
}

std::string toLiteral(double value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

} // namespace

IRProgram IRBuilder::build(const Program& program) {
    tempCounter_ = 0;
    labelCounter_ = 0;

    IRProgram ir;
    ir.name = program.name;
    ir.instructions.push_back(IRInstruction {"PROGRAM", {program.name}});

    for (const auto& declaration : program.declarations) {
        const auto* varDecl = dynamic_cast<const VarDeclStmt*>(declaration.get());
        if (varDecl == nullptr) {
            continue;
        }
        for (const auto& name : varDecl->names) {
            ir.instructions.push_back(IRInstruction {"DECLARE", {name, std::string(valueTypeName(varDecl->type))}});
        }
    }

    for (const auto& statement : program.statements) {
        buildStmt(*statement, ir);
    }

    ir.instructions.push_back(IRInstruction {"END", {}});
    return ir;
}

std::string IRBuilder::buildExpr(const Expr& expr, IRProgram& ir) {
    if (const auto* intExpr = dynamic_cast<const IntExpr*>(&expr)) {
        const std::string temp = newTemp();
        ir.instructions.push_back(IRInstruction {"LOAD_CONST", {temp, std::to_string(intExpr->value)}});
        return temp;
    }

    if (const auto* realExpr = dynamic_cast<const RealExpr*>(&expr)) {
        const std::string temp = newTemp();
        ir.instructions.push_back(IRInstruction {"LOAD_CONST", {temp, toLiteral(realExpr->value)}});
        return temp;
    }

    if (const auto* variableExpr = dynamic_cast<const VariableExpr*>(&expr)) {
        const std::string temp = newTemp();
        ir.instructions.push_back(IRInstruction {"LOAD_VAR", {temp, variableExpr->name}});
        return temp;
    }

    if (const auto* unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) {
        const std::string operand = buildExpr(*unaryExpr->operand, ir);
        const std::string temp = newTemp();
        ir.instructions.push_back(IRInstruction {"NEG", {temp, operand}});
        return temp;
    }

    if (const auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
        const std::string left = buildExpr(*binaryExpr->left, ir);
        const std::string right = buildExpr(*binaryExpr->right, ir);
        const std::string temp = newTemp();
        ir.instructions.push_back(IRInstruction {opcodeForBinary(binaryExpr->op), {temp, left, right}});
        return temp;
    }

    const std::string temp = newTemp();
    ir.instructions.push_back(IRInstruction {"UNKNOWN_EXPR", {temp}});
    return temp;
}

void IRBuilder::buildStmt(const Stmt& stmt, IRProgram& ir) {
    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        const std::string value = buildExpr(*assign->value, ir);
        ir.instructions.push_back(IRInstruction {"STORE", {assign->name, value}});
        return;
    }

    if (const auto* write = dynamic_cast<const WriteStmt*>(&stmt)) {
        const std::string value = buildExpr(*write->expr, ir);
        ir.instructions.push_back(IRInstruction {"PRINT", {value, std::string(valueTypeName(write->expr->inferredType))}});
        return;
    }

    if (const auto* read = dynamic_cast<const ReadStmt*>(&stmt)) {
        ir.instructions.push_back(IRInstruction {"READ", {read->name, std::string(valueTypeName(read->targetType))}});
        return;
    }

    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        const std::string elseLabel = newLabel("else");
        const std::string endLabel = newLabel("endif");
        const std::string condition = buildExpr(*ifStmt->condition, ir);
        ir.instructions.push_back(IRInstruction {"JUMP_IF_FALSE", {condition, elseLabel}});
        for (const auto& child : ifStmt->thenBranch) {
            buildStmt(*child, ir);
        }
        if (!ifStmt->elseBranch.empty()) {
            ir.instructions.push_back(IRInstruction {"JUMP", {endLabel}});
        }
        ir.instructions.push_back(IRInstruction {"LABEL", {elseLabel}});
        for (const auto& child : ifStmt->elseBranch) {
            buildStmt(*child, ir);
        }
        if (!ifStmt->elseBranch.empty()) {
            ir.instructions.push_back(IRInstruction {"LABEL", {endLabel}});
        }
        return;
    }

    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        const std::string startLabel = newLabel("while_start");
        const std::string endLabel = newLabel("while_end");
        ir.instructions.push_back(IRInstruction {"LABEL", {startLabel}});
        const std::string condition = buildExpr(*whileStmt->condition, ir);
        ir.instructions.push_back(IRInstruction {"JUMP_IF_FALSE", {condition, endLabel}});
        for (const auto& child : whileStmt->body) {
            buildStmt(*child, ir);
        }
        ir.instructions.push_back(IRInstruction {"JUMP", {startLabel}});
        ir.instructions.push_back(IRInstruction {"LABEL", {endLabel}});
    }
}

std::string IRBuilder::newTemp() {
    return "%t" + std::to_string(tempCounter_++);
}

std::string IRBuilder::newLabel(const std::string& prefix) {
    return prefix + "_" + std::to_string(labelCounter_++);
}

} // namespace portugol
