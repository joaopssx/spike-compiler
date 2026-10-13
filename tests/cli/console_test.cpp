#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/console.hpp"

namespace {

void test_init_console_switches_output_to_utf8() {
    spike::init_console();
    SPIKE_EXPECT(spike::console_uses_utf8());
}

void test_init_console_can_run_twice() {
    spike::init_console();
    spike::init_console();
    SPIKE_EXPECT(spike::console_uses_utf8());
}

} // namespace

namespace spike_tests {

void run_console_tests() {
    test_init_console_switches_output_to_utf8();
    test_init_console_can_run_twice();
}

} // namespace spike_tests
