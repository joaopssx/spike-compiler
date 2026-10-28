#include "commands/dev_dump.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace spike {

namespace {

constexpr std::size_t kMaxTextPreview = 30;

std::string pad(const std::string& text, std::size_t width) {
    std::size_t shown = 0;
    for (unsigned char c : text) {
        if ((c & 0xC0) != 0x80) ++shown;
    }
    return shown >= width ? text + " " : text + std::string(width - shown, ' ');
}

std::string position(int line, int col) {
    return std::to_string(line) + ":" + std::to_string(col);
}

std::string number_text(const NumberExpr& node) {
    char buffer[40];
    if (node.is_integer) {
        std::snprintf(buffer, sizeof buffer, "%lld", static_cast<long long>(node.value));
    } else {
        std::snprintf(buffer, sizeof buffer, "%g", node.value);
    }
    return buffer;
}

std::string text_preview(const std::string& value) {
    if (value.size() <= kMaxTextPreview) return "\"" + value + "\"";
    return "\"" + value.substr(0, kMaxTextPreview) + "...\"";
}

std::string type_text(const Symbol& symbol) {
    std::string text;
    if (!symbol.dims.empty()) {
        text = "vetor[";
        for (std::size_t i = 0; i < symbol.dims.size(); ++i) {
            if (i > 0) text += ", ";
            text += std::to_string(symbol.dims[i].low) + ".." +
                    std::to_string(symbol.dims[i].high);
        }
        text += "] de ";
    }
    return text + type_name(symbol.type);
}

std::string flags_text(const Symbol& symbol) {
    std::vector<std::string> flags;
    if (symbol.is_const) flags.push_back("constante");
    if (symbol.used) flags.push_back("usada");
    if (symbol.assigned) flags.push_back("atribuída");
    std::string text;
    for (const std::string& flag : flags) text += (text.empty() ? "" : ", ") + flag;
    return text.empty() ? "-" : text;
}

SourcePos expr_pos(const Expr& expr) {
    return std::visit([](const auto& node) { return SourcePos{node.line, node.col}; }, expr.node);
}

} // namespace

std::string dump_tokens(const std::vector<Token>& tokens) {
    std::string out;
    for (const Token& token : tokens) {
        if (token.type == TokenType::END_OF_FILE) continue;
        out += pad("[" + position(token.line, token.col) + "]", 10) +
               pad(token_type_to_string(token.type), 16) + "\"" + token.lexeme + "\"\n";
    }
    return out;
}

std::string dump_symbols(const SymbolTable& symbols) {
    std::string out = pad("nome", 20) + pad("tipo", 28) + pad("posição", 10) + "estado\n";
    for (const Symbol& symbol : symbols) {
        out += pad(symbol.name, 20) + pad(type_text(symbol), 28) +
               pad(position(symbol.line, symbol.col), 10) + flags_text(symbol) + "\n";
    }
    if (symbols.size() == 0) out += "(nenhuma variável, constante ou subprograma declarado)\n";
    return out;
}

std::string describe_expr(const Expr& expr) {
    return std::visit(
        [](const auto& node) -> std::string {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, NumberExpr>) {
                return "número " + number_text(node);
            } else if constexpr (std::is_same_v<T, StringExpr>) {
                return "texto " + text_preview(node.value);
            } else if constexpr (std::is_same_v<T, BoolExpr>) {
                return node.value ? "verdadeiro" : "falso";
            } else if constexpr (std::is_same_v<T, VarExpr>) {
                return "variável " + node.name;
            } else if constexpr (std::is_same_v<T, BinaryExpr> || std::is_same_v<T, UnaryExpr>) {
                return std::string("operação ") + to_string(node.op);
            } else if constexpr (std::is_same_v<T, IndexExpr>) {
                return "índice em " + node.name;
            } else if constexpr (std::is_same_v<T, CallExpr>) {
                return "chamada a " + node.name;
            } else {
                return "erro";
            }
        },
        expr.node);
}

std::string dump_expr_types(const std::unordered_map<const Expr*, TypeKind>& types) {
    struct Row {
        SourcePos pos;
        std::string description;
        std::string type;
    };
    std::vector<Row> rows;
    rows.reserve(types.size());
    for (const auto& [expr, type] : types) {
        rows.push_back(Row{expr_pos(*expr), describe_expr(*expr), type_name(type)});
    }
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        return std::tie(a.pos.line, a.pos.col, a.description, a.type) <
               std::tie(b.pos.line, b.pos.col, b.description, b.type);
    });
    std::string out = pad("posição", 10) + pad("tipo", 12) + "expressão\n";
    for (const Row& row : rows) {
        out += pad(position(row.pos.line, row.pos.col), 10) + pad(row.type, 12) +
               row.description + "\n";
    }
    return out;
}

} // namespace spike
