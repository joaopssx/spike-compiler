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

static void test_wrapping_arithmetic(void) {
    CHECK(spike_add_int(2, 3) == 5);
    CHECK(spike_sub_int(2, 3) == -1);
    CHECK(spike_mul_int(-4, 5) == -20);
    CHECK(spike_add_int(LLONG_MAX, 1) == LLONG_MIN);
    CHECK(spike_sub_int(LLONG_MIN, 1) == LLONG_MAX);
    CHECK(spike_mul_int(LLONG_MAX, 2) == -2);
    CHECK(spike_sub_int(0, LLONG_MIN) == LLONG_MIN);
}

static void test_for_advance(void) {
    spike_int counter = 1;
    CHECK(spike_for_advance(&counter, 2) && counter == 3);
    CHECK(spike_for_advance(&counter, -5) && counter == -2);
    counter = LLONG_MAX - 1;
    CHECK(spike_for_advance(&counter, 1) && counter == LLONG_MAX);
    CHECK(!spike_for_advance(&counter, 1) && counter == LLONG_MAX);
    counter = LLONG_MIN + 2;
    CHECK(!spike_for_advance(&counter, -3) && counter == LLONG_MIN + 2);
    CHECK(spike_for_advance(&counter, -2) && counter == LLONG_MIN);
    spike_check_step(1, 1);
    spike_check_step(-1, 1);
}

static void test_power(void) {
    CHECK(spike_pow(2.0, 10.0, 1) == 1024.0);
    CHECK(spike_pow(2.0, -1.0, 1) == 0.5);
    CHECK(spike_pow(9.0, 0.5, 1) == 3.0);
    CHECK(spike_pow(-2.0, 3.0, 1) == -8.0);
    CHECK(spike_pow(0.0, 0.0, 1) == 1.0);
}

static void test_index(void) {
    spike_str names[3] = {NULL, NULL, NULL};
    CHECK(spike_index(1, 1, 5, 1, "v") == 0);
    CHECK(spike_index(5, 1, 5, 1, "v") == 4);
    CHECK(spike_index(0, 0, 9, 1, "v") == 0);
    CHECK(spike_index(-5, -5, 5, 1, "v") == 0);
    CHECK(spike_index(5, -5, 5, 1, "v") == 10);
    CHECK(spike_index_dim(1, 1, 4, 1, "m", 2) == 0);
    CHECK(spike_index_dim(4, 1, 4, 1, "m", 2) == 3);
    CHECK(spike_index_dim(0, -1, 1, 1, "m", 3) == 1);
    spike_fill_str(names, 3);
    CHECK(spike_str_eq(names[0], "") && spike_str_eq(names[2], ""));
}

static void test_call_depth(void) {
    spike_enter("f", 1);
    spike_enter("f", 1);
    CHECK(spike_call_depth == 2);
    spike_leave();
    spike_leave();
    spike_leave();
    CHECK(spike_call_depth == 0);
    CHECK(spike_stack_budget > 0);
}

static void test_saved_calls(void) {
    int i;
    for (i = 0; i < 70; ++i) spike_enter(i == 69 ? "fundo" : "f", i + 1);
    CHECK(spike_call_depth == 70);
    CHECK(spike_calls[69 % SPIKE_SAVED_CALLS].depth == 70);
    CHECK(strcmp(spike_calls[69 % SPIKE_SAVED_CALLS].name, "fundo") == 0);
    CHECK(spike_calls[69 % SPIKE_SAVED_CALLS].line == 70);
    CHECK(spike_calls[2].depth == 67);
    for (i = 0; i < 70; ++i) spike_leave();
    CHECK(spike_call_depth == 0);
}

static void test_error_colors_and_hints(void) {
    int code;
    CHECK(spike_color_wanted(NULL, 1));
    CHECK(spike_color_wanted("", 1));
    CHECK(!spike_color_wanted("1", 1));
    CHECK(!spike_color_wanted(NULL, 0));
    for (code = SPIKE_R_DIV_ZERO; code <= SPIKE_R_TEXT_RANGE; ++code) {
        CHECK(spike_hint(code) != NULL);
    }
    CHECK(spike_hint(0) == NULL);
    CHECK(strstr(spike_hint(SPIKE_R_DIV_ZERO), "dividir") != NULL);
}

static void test_math(void) {
    CHECK(spike_abs_int(-5) == 5 && spike_abs_int(7) == 7);
    CHECK(spike_abs_int(LLONG_MIN) == LLONG_MIN);
    CHECK(spike_sqrt(16.0, 1) == 4.0);
    CHECK(fabs(spike_log(1000.0, 1) - 3.0) < 1e-12);
    CHECK(spike_round(2.4, 1) == 2 && spike_round(2.5, 1) == 3 && spike_round(-2.5, 1) == -3);
    CHECK(spike_trunc(-2.7, 1) == -2 && spike_trunc(2.7, 1) == 2);
    CHECK(spike_floor(-2.1, 1) == -3 && spike_floor(2.9, 1) == 2);
    CHECK(spike_ceil(-2.9, 1) == -2 && spike_ceil(2.1, 1) == 3);
    CHECK(spike_real_to_int(9.2e18, 1) == 9200000000000000000LL);
    CHECK(spike_real_to_int(-9223372036854775808.0, 1) == LLONG_MIN);
}

static void test_random(void) {
    int i;
    spike_seed_random(42);
    CHECK(spike_random(1, 1000000, 1) == 275414);
    CHECK(spike_random(-5, -5, 1) == -5);
    spike_seed_random(42);
    CHECK(spike_random(1, 6, 1) == 2 && spike_random(1, 6, 1) == 2 &&
          spike_random(1, 6, 1) == 1 && spike_random(1, 6, 1) == 1 &&
          spike_random(1, 6, 1) == 5);
    for (i = 0; i < 1000; ++i) {
        spike_int value = spike_random(-3, 3, 1);
        CHECK(value >= -3 && value <= 3);
    }
    (void)spike_random(LLONG_MIN, LLONG_MAX, 1);
}

static void test_arithmetic(void) {
    CHECK(spike_div_int(7, 2, 1) == 3);
    CHECK(spike_div_int(-7, 2, 1) == -3);
    CHECK(spike_div_int(7, -2, 1) == -3);
    CHECK(spike_mod_int(7, -3, 1) == 1);
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
    test_wrapping_arithmetic();
    test_for_advance();
    test_arithmetic();
    test_index();
    test_call_depth();
    test_saved_calls();
    test_error_colors_and_hints();
    test_math();
    test_random();
    test_power();
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
