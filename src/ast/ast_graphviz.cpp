#include "ast_graphviz.hpp"

#include <sstream>

namespace portugol {

namespace {

class GraphBuilder {
public:
    std::string build(const Program& program) {
        out_ << "digraph SpikeAST {\n";
        out_ << "  rankdir=TB;\n";
        out_ << "  node [shape=box, style=rounded];\n";

        const int programId = nextId();
        emitNode(programId, "Program\\n" + escape(program.name));

        const int declarationsId = nextId();
        emitNode(declarationsId, "Declarations");
        emitEdge(programId, declarationsId);
        for (const auto& declaration : program.declarations) {
            emitStmt(*declaration, declarationsId);
        }

        const int statementsId = nextId();
        emitNode(statementsId, "Statements");
        emitEdge(programId, statementsId);
        for (const auto& statement : program.statements) {
            emitStmt(*statement, statementsId);
        }

        out_ << "}\n";
        return out_.str();
    }

private:
    int nextId() {
        return currentId_++;
    }

    static std::string escape(std::string_view text) {
        std::string escaped;
        for (const char ch : text) {
            if (ch == '"') {
                escaped += "\\\"";
            } else if (ch == '\n') {
                escaped += "\\n";
            } else {
                escaped.push_back(ch);
            }
        }
        return escaped;
    }

    void emitNode(int id, const std::string& label) {
        out_ << "  node" << id << " [label=\"" << escape(label) << "\"];\n";
    }

    void emitEdge(int from, int to) {
        out_ << "  node" << from << " -> node" << to << ";\n";
    }

    void emitExpr(const Expr& expr, int parentId) {
        const int id = nextId();

        if (const auto* intExpr = dynamic_cast<const IntExpr*>(&expr)) {
            emitNode(id, "IntExpr\\n" + std::to_string(intExpr->value));
        } else if (const auto* realExpr = dynamic_cast<const RealExpr*>(&expr)) {
            emitNode(id, "RealExpr\\n" + std::to_string(realExpr->value));
        } else if (const auto* variableExpr = dynamic_cast<const VariableExpr*>(&expr)) {
            emitNode(id, "VariableExpr\\n" + variableExpr->name);
        } else if (const auto* unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) {
            emitNode(id, "UnaryExpr\\n" + unaryExpr->op);
            emitEdge(parentId, id);
            emitExpr(*unaryExpr->operand, id);
            return;
        } else if (const auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
            emitNode(id, "BinaryExpr\\n" + binaryExpr->op);
            emitEdge(parentId, id);
            emitExpr(*binaryExpr->left, id);
            emitExpr(*binaryExpr->right, id);
            return;
        } else {
            emitNode(id, "UnknownExpr");
        }

        emitEdge(parentId, id);
    }

    void emitStmt(const Stmt& stmt, int parentId) {
        const int id = nextId();

        if (const auto* decl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
            std::string label = "VarDecl\\n" + std::string(valueTypeName(decl->type));
            for (const auto& name : decl->names) {
                label += "\\n" + name;
            }
            emitNode(id, label);
            emitEdge(parentId, id);
            return;
        }

        if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
            emitNode(id, "AssignStmt\\n" + assign->name);
            emitEdge(parentId, id);
            emitExpr(*assign->value, id);
            return;
        }

        if (const auto* write = dynamic_cast<const WriteStmt*>(&stmt)) {
            emitNode(id, "WriteStmt");
            emitEdge(parentId, id);
            emitExpr(*write->expr, id);
            return;
        }

        if (const auto* read = dynamic_cast<const ReadStmt*>(&stmt)) {
            emitNode(id, "ReadStmt\\n" + read->name);
            emitEdge(parentId, id);
            return;
        }

        if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
            emitNode(id, "IfStmt");
            emitEdge(parentId, id);
            emitExpr(*ifStmt->condition, id);
            for (const auto& child : ifStmt->thenBranch) {
                emitStmt(*child, id);
            }
            for (const auto& child : ifStmt->elseBranch) {
                emitStmt(*child, id);
            }
            return;
        }

        if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
            emitNode(id, "WhileStmt");
            emitEdge(parentId, id);
            emitExpr(*whileStmt->condition, id);
            for (const auto& child : whileStmt->body) {
                emitStmt(*child, id);
            }
            return;
        }

        emitNode(id, "UnknownStmt");
        emitEdge(parentId, id);
    }

    std::ostringstream out_;
    int currentId_ {0};
};

} // namespace

std::string astToGraphviz(const Program& program) {
    GraphBuilder builder;
    return builder.build(program);
}

} // namespace portugol
