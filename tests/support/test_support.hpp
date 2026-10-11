#ifndef SPIKE_TESTS_TEST_SUPPORT_HPP
#define SPIKE_TESTS_TEST_SUPPORT_HPP

#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace spike_tests {

inline int g_failures = 0;

inline std::vector<spike::Token> tokens_of(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    std::vector<spike::Token> toks = lex.tokenize();
    if (!toks.empty() && toks.back().type == spike::TokenType::END_OF_FILE) {
        toks.pop_back();
    }
    return toks;
}

} // namespace spike_tests

#define SPIKE_EXPECT(cond)                                                     \
    do {                                                                       \
        if (!(cond)) {                                                         \
            ++::spike_tests::g_failures;                                       \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__               \
                      << ": " #cond << std::endl;                              \
        }                                                                      \
    } while (0)

#endif // SPIKE_TESTS_TEST_SUPPORT_HPP
