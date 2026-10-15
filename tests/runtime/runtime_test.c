#include "spike_runtime.h"

#include <limits.h>

static int failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            ++failures;                                                        \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
        }                                                                      \
    } while (0)

static void check_real(spike_real value, const char* expected) {
    char buffer[64];
    spike_format_real(value, buffer, sizeof buffer);
    if (strcmp(buffer, expected) != 0) {
        ++failures;
        fprintf(stderr, "FAIL: real formatted as '%s', expected '%s'\n", buffer, expected);
    }
}

static void test_real_format(void) {
    check_real(3.0, "3");
    check_real(3.5, "3.5");
    check_real(0.1 + 0.2, "0.3");
    check_real(-2.25, "-2.25");
    check_real(0.0, "0");
    check_real(-0.0, "0");
    check_real(1.0 / 3.0, "0.3333333333");
    check_real(7.0 / 2.0, "3.5");
    check_real(1e20, "1e+20");
}

static void test_strings(void) {
    CHECK(spike_str_eq(spike_str_concat("Olá, ", "mundo"), "Olá, mundo"));
    CHECK(spike_str_eq(spike_str_concat("", ""), ""));
    CHECK(spike_str_eq(spike_str_copy("texto"), "texto"));
    CHECK(!spike_str_eq("a", "b"));
    CHECK(spike_str_cmp("a", "b") == -1);
    CHECK(spike_str_cmp("b", "a") == 1);
    CHECK(spike_str_cmp("abc", "abc") == 0);
    CHECK(spike_str_cmp("abc", "abcd") == -1);
}

static void test_arithmetic(void) {
    CHECK(spike_div_int(7, 2, 1) == 3);
    CHECK(spike_div_int(-7, 2, 1) == -3);
    CHECK(spike_mod_int(7, 3, 1) == 1);
    CHECK(spike_mod_int(-7, 3, 1) == -1);
    CHECK(spike_div_int(LLONG_MIN, -1, 1) == LLONG_MIN);
    CHECK(spike_mod_int(LLONG_MIN, -1, 1) == 0);
    CHECK(spike_div_real(7.0, 2.0, 1) == 3.5);
}

static void test_parse_int(void) {
    spike_int value = 0;
    CHECK(spike_parse_int("42", &value) && value == 42);
    CHECK(spike_parse_int("  -7 ", &value) && value == -7);
    CHECK(spike_parse_int("+3", &value) && value == 3);
    CHECK(spike_parse_int("9223372036854775807", &value) && value == LLONG_MAX);
    CHECK(!spike_parse_int("99999999999999999999", &value));
    CHECK(!spike_parse_int("abc", &value));
    CHECK(!spike_parse_int("", &value));
    CHECK(!spike_parse_int("-", &value));
    CHECK(!spike_parse_int("1.5", &value));
    CHECK(!spike_parse_int("12abc", &value));
}

static void test_parse_real(void) {
    spike_real value = 0.0;
    CHECK(spike_parse_real("3.5", &value) && value == 3.5);
    CHECK(spike_parse_real("3,5", &value) && value == 3.5);
    CHECK(spike_parse_real(" -2 ", &value) && value == -2.0);
    CHECK(spike_parse_real("10", &value) && value == 10.0);
    CHECK(!spike_parse_real("1e5", &value));
    CHECK(!spike_parse_real(".", &value));
    CHECK(!spike_parse_real("1.2.3", &value));
    CHECK(!spike_parse_real("abc", &value));
    CHECK(!spike_parse_real("", &value));
}

static void test_parse_bool(void) {
    spike_bool value = 0;
    CHECK(spike_parse_bool("verdadeiro", &value) && value == 1);
    CHECK(spike_parse_bool(" V ", &value) && value == 1);
    CHECK(spike_parse_bool("Sim", &value) && value == 1);
    CHECK(spike_parse_bool("FALSO", &value) && value == 0);
    CHECK(spike_parse_bool("f", &value) && value == 0);
    CHECK(spike_parse_bool("nao", &value) && value == 0);
    CHECK(!spike_parse_bool("talvez", &value));
    CHECK(!spike_parse_bool("", &value));
}

int main(void) {
    spike_init();
    test_real_format();
    test_strings();
    test_arithmetic();
    test_parse_int();
    test_parse_real();
    test_parse_bool();
    if (failures == 0) {
        printf("runtime tests passed\n");
        return 0;
    }
    fprintf(stderr, "%d runtime check(s) failed\n", failures);
    return 1;
}
