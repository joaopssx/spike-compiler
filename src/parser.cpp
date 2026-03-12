#include "parser.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

#include "utils.hpp"

namespace portugol {

namespace {

bool isEndToken(const Token& token, const std::vector<TokenType>& endTokens) {
    return std::find(endTokens.begin(), endTokens.end(), token.type) != endTokens.end();
}

std::string indent(std::size_t level) {
    return std::string(level * 2, ' ');
}

std::string formatReal(double value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

void dumpExpr(const Expr& expr, std::size_t level, std::ostringstream& out);
void dumpStmt(const Stmt& stmt, std::size_t level, std::ostringstream& out);

void dumpExpr(const Expr& expr, std::size_t level, std::ostringstream& out) {
    if (const auto* intExpr = dynamic_cast<const IntExpr*>(&expr)) {
        out << indent(level) << "Inteiro(" << intExpr->value << ")\n";
        return;
    }

    if (const auto* realExpr = dynamic_cast<const RealExpr*>(&expr)) {
        out << indent(level) << "Real(" << formatReal(realExpr->value) << ")\n";
        return;
    }

    if (const auto* variableExpr = dynamic_cast<const VariableExpr*>(&expr)) {
        out << indent(level) << "Variavel(" << variableExpr->name << ")\n";
        return;
    }

    if (const auto* unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) {
        out << indent(level) << "Unario(" << unaryExpr->op << ")\n";
        dumpExpr(*unaryExpr->operand, level + 1, out);
        return;
    }

    if (const auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
        out << indent(level) << "Binario(" << binaryExpr->op << ")\n";
        dumpExpr(*binaryExpr->left, level + 1, out);
        dumpExpr(*binaryExpr->right, level + 1, out);
        return;
    }

    out << indent(level) << "ExpressaoDesconhecida\n";
}

void dumpStmt(const Stmt& stmt, std::size_t level, std::ostringstream& out) {
    if (const auto* decl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        out << indent(level) << "Declaracao(" << valueTypeName(decl->type) << ": ";
        for (std::size_t index = 0; index < decl->names.size(); ++index) {
            if (index > 0) {
                out << ", ";
            }
            out << decl->names[index];
        }
        out << ")\n";
        return;
    }

    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        out << indent(level) << "Atribuicao(" << assign->name << ")\n";
        dumpExpr(*assign->value, level + 1, out);
        return;
    }

    if (const auto* write = dynamic_cast<const WriteStmt*>(&stmt)) {
        out << indent(level) << "Escreva\n";
        dumpExpr(*write->expr, level + 1, out);
        return;
    }

    if (const auto* read = dynamic_cast<const ReadStmt*>(&stmt)) {
        out << indent(level) << "Leia(" << read->name << ")\n";
        return;
    }

    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        out << indent(level) << "Se\n";
        out << indent(level + 1) << "Condicao\n";
        dumpExpr(*ifStmt->condition, level + 2, out);
        out << indent(level + 1) << "Entao\n";
        for (const auto& child : ifStmt->thenBranch) {
            dumpStmt(*child, level + 2, out);
        }
        if (!ifStmt->elseBranch.empty()) {
            out << indent(level + 1) << "Senao\n";
            for (const auto& child : ifStmt->elseBranch) {
                dumpStmt(*child, level + 2, out);
            }
        }
        return;
    }

    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        out << indent(level) << "Enquanto\n";
        out << indent(level + 1) << "Condicao\n";
        dumpExpr(*whileStmt->condition, level + 2, out);
        out << indent(level + 1) << "Corpo\n";
        for (const auto& child : whileStmt->body) {
            dumpStmt(*child, level + 2, out);
        }
        return;
    }

    out << indent(level) << "ComandoDesconhecido\n";
}

} // namespace

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens) {
}

Program Parser::parseProgram() {
    const Token& algoritmoToken = expect(TokenType::Algoritmo, "esperava a palavra-chave 'algoritmo'");
    const Token& nameToken = expect(TokenType::String, "esperava o nome do algoritmo entre aspas");

    Program program(algoritmoToken.location, nameToken.lexeme);
    program.declarations = parseVarBlock();

    expect(TokenType::Inicio, "esperava a palavra-chave 'inicio'");
    program.statements = parseStatementListUntil({TokenType::FimAlgoritmo});
    expect(TokenType::FimAlgoritmo, "esperava a palavra-chave 'fimalgoritmo'");
    expect(TokenType::EndOfFile, "esperava o fim do arquivo");

    return program;
}

const Token& Parser::current() const {
    return tokens_[pos_];
}

const Token& Parser::previous() const {
    return tokens_[pos_ - 1];
}

bool Parser::isAtEnd() const {
    return current().type == TokenType::EndOfFile;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++pos_;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return type == TokenType::EndOfFile;
    }

    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }

    advance();
    return true;
}

const Token& Parser::expect(TokenType type, const std::string& message) {
    if (!check(type)) {
        throw CompilerError("parser", message, current().location);
    }

    return advance();
}

std::vector<std::unique_ptr<Stmt>> Parser::parseVarBlock() {
    std::vector<std::unique_ptr<Stmt>> declarations;
    if (!match(TokenType::Var)) {
        return declarations;
    }

    while (check(TokenType::Identifier)) {
        declarations.push_back(parseDeclaration());
    }

    return declarations;
}

std::unique_ptr<Stmt> Parser::parseDeclaration() {
    const SourceLocation location = current().location;
    std::vector<std::string> names = parseIdentifierList();
    expect(TokenType::Colon, "esperava ':' apos a lista de variaveis");
    return std::make_unique<VarDeclStmt>(location, std::move(names), parseType());
}

std::vector<std::string> Parser::parseIdentifierList() {
    std::vector<std::string> names;
    names.push_back(expect(TokenType::Identifier, "esperava um identificador").lexeme);

    while (match(TokenType::Comma)) {
        names.push_back(expect(TokenType::Identifier, "esperava um identificador apos ','").lexeme);
    }

    return names;
}

ValueType Parser::parseType() {
    if (match(TokenType::Inteiro)) {
        return ValueType::Inteiro;
    }
    if (match(TokenType::Real)) {
        return ValueType::Real;
    }
    if (match(TokenType::Logico)) {
        return ValueType::Logico;
    }
    if (match(TokenType::Caractere)) {
        return ValueType::Caractere;
    }

    throw CompilerError("parser", "esperava um tipo valido ('inteiro', 'real', 'logico' ou 'caractere')", current().location);
}

std::vector<std::unique_ptr<Stmt>> Parser::parseStatementListUntil(const std::vector<TokenType>& endTokens) {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!isAtEnd() && !isEndToken(current(), endTokens)) {
        statements.push_back(parseStatement());
    }

    return statements;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::Identifier)) {
        return parseAssignment();
    }
    if (match(TokenType::Escreva)) {
        return parseWrite();
    }
    if (match(TokenType::Leia)) {
        return parseRead();
    }
    if (match(TokenType::Se)) {
        return parseIf();
    }
    if (match(TokenType::Enquanto)) {
        return parseWhile();
    }

    throw CompilerError("parser", "comando invalido", current().location);
}

std::unique_ptr<Stmt> Parser::parseAssignment() {
    const Token& identifier = expect(TokenType::Identifier, "esperava um identificador");
    expect(TokenType::Assign, "esperava '<-' na atribuicao");
    return std::make_unique<AssignStmt>(identifier.location, identifier.lexeme, parseExpression());
}

std::unique_ptr<Stmt> Parser::parseWrite() {
    const Token& keyword = previous();
    expect(TokenType::LParen, "esperava '('");
    auto expr = parseExpression();
    expect(TokenType::RParen, "esperava ')'");
    return std::make_unique<WriteStmt>(keyword.location, std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseRead() {
    const Token& keyword = previous();
    expect(TokenType::LParen, "esperava '('");
    const Token& identifier = expect(TokenType::Identifier, "esperava um identificador dentro de leia(...)");
    expect(TokenType::RParen, "esperava ')'");
    return std::make_unique<ReadStmt>(keyword.location, identifier.lexeme);
}

std::unique_ptr<Stmt> Parser::parseIf() {
    const Token& keyword = previous();
    auto stmt = std::make_unique<IfStmt>(keyword.location);
    stmt->condition = parseExpression();
    expect(TokenType::Entao, "esperava a palavra-chave 'entao'");
    stmt->thenBranch = parseStatementListUntil({TokenType::Senao, TokenType::FimSe});

    if (match(TokenType::Senao)) {
        stmt->elseBranch = parseStatementListUntil({TokenType::FimSe});
    }

    expect(TokenType::FimSe, "esperava a palavra-chave 'fimse'");
    return stmt;
}

std::unique_ptr<Stmt> Parser::parseWhile() {
    const Token& keyword = previous();
    auto stmt = std::make_unique<WhileStmt>(keyword.location);
    stmt->condition = parseExpression();
    expect(TokenType::Faca, "esperava a palavra-chave 'faca'");
    stmt->body = parseStatementListUntil({TokenType::FimEnquanto});
    expect(TokenType::FimEnquanto, "esperava a palavra-chave 'fimenquanto'");
    return stmt;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();

    while (check(TokenType::Equal) || check(TokenType::NotEqual)) {
        const Token op = advance();
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(op.location, op.lexeme, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();

    while (check(TokenType::Less) || check(TokenType::LessEqual) || check(TokenType::Greater) || check(TokenType::GreaterEqual)) {
        const Token op = advance();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(op.location, op.lexeme, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();

    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        const Token op = advance();
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(op.location, op.lexeme, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();

    while (check(TokenType::Star) || check(TokenType::Slash)) {
        const Token op = advance();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(op.location, op.lexeme, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::Minus)) {
        const Token& op = previous();
        return std::make_unique<UnaryExpr>(op.location, op.lexeme, parseUnary());
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::NumberInt)) {
        const Token& token = previous();
        return std::make_unique<IntExpr>(token.location, std::stoi(token.lexeme));
    }
    if (match(TokenType::NumberReal)) {
        const Token& token = previous();
        return std::make_unique<RealExpr>(token.location, std::stod(token.lexeme));
    }
    if (match(TokenType::Identifier)) {
        const Token& token = previous();
        return std::make_unique<VariableExpr>(token.location, token.lexeme);
    }
    if (match(TokenType::LParen)) {
        auto expr = parseExpression();
        expect(TokenType::RParen, "esperava ')' apos a expressao");
        return expr;
    }

    throw CompilerError("parser", "expressao invalida", current().location);
}

std::string dumpAst(const Program& program) {
    std::ostringstream out;
    out << "Programa(\"" << program.name << "\")\n";

    out << "  Declaracoes\n";
    if (program.declarations.empty()) {
        out << "    <vazio>\n";
    } else {
        for (const auto& declaration : program.declarations) {
            dumpStmt(*declaration, 2, out);
        }
    }

    out << "  Comandos\n";
    if (program.statements.empty()) {
        out << "    <vazio>\n";
    } else {
        for (const auto& stmt : program.statements) {
            dumpStmt(*stmt, 2, out);
        }
    }

    return out.str();
}

} // namespace portugol
