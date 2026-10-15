#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/runtime_text.hpp"

#include <cstring>
#include <string>

namespace {

void test_runtime_source_is_embedded() {
    const std::string source(spike::runtime_source(), spike::runtime_source_size());
    SPIKE_EXPECT(!source.empty());
    SPIKE_EXPECT(std::strlen(spike::runtime_source()) == spike::runtime_source_size());
    SPIKE_EXPECT(source.find("#ifndef SPIKE_RUNTIME_H") == 0);
    SPIKE_EXPECT(source.find("spike_write_int") != std::string::npos);
    SPIKE_EXPECT(source.find("spike_fail") != std::string::npos);
}

void test_runtime_source_has_unix_line_endings() {
    const std::string source(spike::runtime_source(), spike::runtime_source_size());
    SPIKE_EXPECT(source.find('\r') == std::string::npos);
    SPIKE_EXPECT(source.find("execução") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_runtime_text_tests() {
    test_runtime_source_is_embedded();
    test_runtime_source_has_unix_line_endings();
}

} // namespace spike_tests
