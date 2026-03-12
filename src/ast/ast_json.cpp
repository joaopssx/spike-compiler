#include "ast_json.hpp"

#include <sstream>

namespace portugol {

namespace {

std::string jsonEscape(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size() + 8);

    for (const char ch : text) {
        switch (ch) {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }

    return escaped;
}

std::string indent(std::size_t level) {
    return std::string(level * 2, ' ');
}

void writeExprJson(const Expr& expr, std::ostringstream& out, std::size_t level);
void writeStmtJson(const Stmt& stmt, std::ostringstream& out, std::size_t level);

void writeLocationJson(const SourceLocation& location, std::ostringstream& out, std::size_t level) {
    out << indent(level) << "\"location\": {\n";
    out << indent(level + 1) << "\"line\": " << location.line << ",\n";
    out << indent(level + 1) << "\"column\": " << location.column << ",\n";
    out << indent(level + 1) << "\"offset\": " << location.offset << "\n";
    out << indent(level) << "}";
}

void writeExprJson(const Expr& expr, std::ostringstream& out, std::size_t level) {
    out << indent(level) << "{\n";

    if (const auto* intExpr = dynamic_cast<const IntExpr*>(&expr)) {
        out << indent(level + 1) << "\"kind\": \"IntExpr\",\n";
        out << indent(level + 1) << "\"value\": " << intExpr->value << ",\n";
    } else if (const auto* realExpr = dynamic_cast<const RealExpr*>(&expr)) {
        out << indent(level + 1) << "\"kind\": \"RealExpr\",\n";
        out << indent(level + 1) << "\"value\": " << realExpr->value << ",\n";
    } else if (const auto* variableExpr = dynamic_cast<const VariableExpr*>(&expr)) {
        out << indent(level + 1) << "\"kind\": \"VariableExpr\",\n";
        out << indent(level + 1) << "\"name\": \"" << jsonEscape(variableExpr->name) << "\",\n";
    } else if (const auto* unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) {
        out << indent(level + 1) << "\"kind\": \"UnaryExpr\",\n";
        out << indent(level + 1) << "\"operator\": \"" << jsonEscape(unaryExpr->op) << "\",\n";
        out << indent(level + 1) << "\"operand\": \n";
        writeExprJson(*unaryExpr->operand, out, level + 2);
        out << ",\n";
    } else if (const auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
        out << indent(level + 1) << "\"kind\": \"BinaryExpr\",\n";
        out << indent(level + 1) << "\"operator\": \"" << jsonEscape(binaryExpr->op) << "\",\n";
        out << indent(level + 1) << "\"left\": \n";
        writeExprJson(*binaryExpr->left, out, level + 2);
        out << ",\n";
        out << indent(level + 1) << "\"right\": \n";
        writeExprJson(*binaryExpr->right, out, level + 2);
        out << ",\n";
    } else {
        out << indent(level + 1) << "\"kind\": \"UnknownExpr\",\n";
    }

    out << indent(level + 1) << "\"inferredType\": \"" << valueTypeName(expr.inferredType) << "\",\n";
    writeLocationJson(expr.location, out, level + 1);
    out << "\n" << indent(level) << "}";
}

void writeStmtArrayJson(const std::vector<std::unique_ptr<Stmt>>& statements, std::ostringstream& out, std::size_t level) {
    out << indent(level) << "[\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        writeStmtJson(*statements[index], out, level + 1);
        if (index + 1 < statements.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << indent(level) << "]";
}

void writeStmtJson(const Stmt& stmt, std::ostringstream& out, std::size_t level) {
    out << indent(level) << "{\n";

    if (const auto* decl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        out << indent(level + 1) << "\"kind\": \"VarDeclStmt\",\n";
        out << indent(level + 1) << "\"type\": \"" << valueTypeName(decl->type) << "\",\n";
        out << indent(level + 1) << "\"names\": [";
        for (std::size_t index = 0; index < decl->names.size(); ++index) {
            if (index > 0) {
                out << ", ";
            }
            out << "\"" << jsonEscape(decl->names[index]) << "\"";
        }
        out << "],\n";
    } else if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        out << indent(level + 1) << "\"kind\": \"AssignStmt\",\n";
        out << indent(level + 1) << "\"name\": \"" << jsonEscape(assign->name) << "\",\n";
        out << indent(level + 1) << "\"value\": \n";
        writeExprJson(*assign->value, out, level + 2);
        out << ",\n";
    } else if (const auto* write = dynamic_cast<const WriteStmt*>(&stmt)) {
        out << indent(level + 1) << "\"kind\": \"WriteStmt\",\n";
        out << indent(level + 1) << "\"expr\": \n";
        writeExprJson(*write->expr, out, level + 2);
        out << ",\n";
    } else if (const auto* read = dynamic_cast<const ReadStmt*>(&stmt)) {
        out << indent(level + 1) << "\"kind\": \"ReadStmt\",\n";
        out << indent(level + 1) << "\"name\": \"" << jsonEscape(read->name) << "\",\n";
        out << indent(level + 1) << "\"targetType\": \"" << valueTypeName(read->targetType) << "\",\n";
    } else if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        out << indent(level + 1) << "\"kind\": \"IfStmt\",\n";
        out << indent(level + 1) << "\"condition\": \n";
        writeExprJson(*ifStmt->condition, out, level + 2);
        out << ",\n";
        out << indent(level + 1) << "\"thenBranch\": \n";
        writeStmtArrayJson(ifStmt->thenBranch, out, level + 2);
        out << ",\n";
        out << indent(level + 1) << "\"elseBranch\": \n";
        writeStmtArrayJson(ifStmt->elseBranch, out, level + 2);
        out << ",\n";
    } else if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        out << indent(level + 1) << "\"kind\": \"WhileStmt\",\n";
        out << indent(level + 1) << "\"condition\": \n";
        writeExprJson(*whileStmt->condition, out, level + 2);
        out << ",\n";
        out << indent(level + 1) << "\"body\": \n";
        writeStmtArrayJson(whileStmt->body, out, level + 2);
        out << ",\n";
    } else {
        out << indent(level + 1) << "\"kind\": \"UnknownStmt\",\n";
    }

    writeLocationJson(stmt.location, out, level + 1);
    out << "\n" << indent(level) << "}";
}

} // namespace

std::string astToJson(const Program& program) {
    std::ostringstream out;
    out << "{\n";
    out << indent(1) << "\"kind\": \"Program\",\n";
    out << indent(1) << "\"name\": \"" << jsonEscape(program.name) << "\",\n";
    out << indent(1) << "\"declarations\": \n";
    writeStmtArrayJson(program.declarations, out, 2);
    out << ",\n";
    out << indent(1) << "\"statements\": \n";
    writeStmtArrayJson(program.statements, out, 2);
    out << ",\n";
    writeLocationJson(program.location, out, 1);
    out << "\n}\n";
    return out.str();
}

} // namespace portugol
