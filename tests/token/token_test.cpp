#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/token.hpp"

#include <string>

namespace {

// ---- token -----------------------------------------------------------------

void test_token_type_to_string_known_values() {
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::NUMBER) == "NUMBER");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::ALGORITMO) == "ALGORITMO");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::FIM_ENQUANTO) == "FIM_ENQUANTO");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::ARROW) == "ARROW");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::END_OF_FILE) == "END_OF_FILE");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::UNKNOWN) == "UNKNOWN");
}

void test_token_default_construction() {
    spike::Token t{spike::TokenType::IDENTIFIER, "contador", 3, 7};
    SPIKE_EXPECT(t.type == spike::TokenType::IDENTIFIER);
    SPIKE_EXPECT(t.lexeme == "contador");
    SPIKE_EXPECT(t.line == 3);
    SPIKE_EXPECT(t.col == 7);
}

} // namespace

namespace spike_tests {

void run_token_tests() {
    test_token_type_to_string_known_values();
    test_token_default_construction();
}

} // namespace spike_tests
