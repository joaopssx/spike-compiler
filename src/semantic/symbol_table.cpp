#include "spike/symbol_table.hpp"

#include <cctype>

namespace spike {

std::string symbol_key(const std::string& name) {
    std::string key;
    key.reserve(name.size());
    for (unsigned char c : name) {
        key.push_back(static_cast<char>(std::tolower(c)));
    }
    return key;
}

bool SymbolTable::declare(const std::string& name, TypeKind type, int line, int col,
                          bool is_const) {
    const std::string key = symbol_key(name);
    Scope& scope = scopes_.back();
    if (scope.count(key) != 0) return false;
    scope.emplace(key, symbols_.size());
    symbols_.push_back(Symbol{name, type, line, col, false, false, is_const, {}});
    return true;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    const std::string key = symbol_key(name);
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        const auto it = scope->find(key);
        if (it != scope->end()) return &symbols_[it->second];
    }
    return nullptr;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    return const_cast<SymbolTable*>(this)->lookup(name);
}

const Symbol* SymbolTable::lookup_global(const std::string& name) const {
    const auto it = scopes_.front().find(symbol_key(name));
    return it == scopes_.front().end() ? nullptr : &symbols_[it->second];
}

void SymbolTable::push_scope() {
    scopes_.emplace_back();
}

void SymbolTable::pop_scope() {
    if (scopes_.size() > 1) scopes_.pop_back();
}

std::size_t SymbolTable::depth() const {
    return scopes_.size();
}

std::size_t SymbolTable::size() const {
    return symbols_.size();
}

std::deque<Symbol>::const_iterator SymbolTable::begin() const {
    return symbols_.begin();
}

std::deque<Symbol>::const_iterator SymbolTable::end() const {
    return symbols_.end();
}

} // namespace spike
