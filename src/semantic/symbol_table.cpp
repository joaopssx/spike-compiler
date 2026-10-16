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
    if (index_.count(key) != 0) return false;
    index_.emplace(key, symbols_.size());
    symbols_.push_back(Symbol{name, type, line, col, false, false, is_const, {}});
    return true;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    const auto it = index_.find(symbol_key(name));
    return it == index_.end() ? nullptr : &symbols_[it->second];
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    const auto it = index_.find(symbol_key(name));
    return it == index_.end() ? nullptr : &symbols_[it->second];
}

std::size_t SymbolTable::size() const {
    return symbols_.size();
}

std::vector<Symbol>::const_iterator SymbolTable::begin() const {
    return symbols_.begin();
}

std::vector<Symbol>::const_iterator SymbolTable::end() const {
    return symbols_.end();
}

} // namespace spike
