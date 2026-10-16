#ifndef SPIKE_SYMBOL_TABLE_HPP
#define SPIKE_SYMBOL_TABLE_HPP

#include "spike/ast_enums.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace spike {

struct ArrayBounds {
    long long low = 0;
    long long high = 0;
};

struct Symbol {
    std::string name;
    TypeKind type = TypeKind::None;
    int line = 0;
    int col = 0;
    bool used = false;
    bool assigned = false;
    bool is_const = false;
    std::vector<ArrayBounds> dims;
};

std::string symbol_key(const std::string& name);

class SymbolTable {
public:
    bool declare(const std::string& name, TypeKind type, int line, int col,
                 bool is_const = false);

    Symbol* lookup(const std::string& name);
    const Symbol* lookup(const std::string& name) const;

    std::size_t size() const;
    std::vector<Symbol>::const_iterator begin() const;
    std::vector<Symbol>::const_iterator end() const;

private:
    std::vector<Symbol> symbols_;
    std::unordered_map<std::string, std::size_t> index_;
};

} // namespace spike

#endif // SPIKE_SYMBOL_TABLE_HPP
