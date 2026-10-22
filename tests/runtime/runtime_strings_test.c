#include "spike_runtime.h"

static int failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            ++failures;                                                        \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
        }                                                                      \
    } while (0)

#define CHECK_STR(actual, expected) CHECK(strcmp((actual), (expected)) == 0)

static void test_length(void) {
    CHECK(spike_str_length("") == 0);
    CHECK(spike_str_length("ação") == 4);
    CHECK(spike_str_length("ÁRVORE") == 6);
    CHECK(spike_str_length("\xF0\x9F\x98\x80") == 1);
    CHECK(spike_str_length("a\xF0\x9F\x98\x80" "b") == 3);
    CHECK(spike_str_length("\xFF\xFE") == 2);
    CHECK(spike_str_length("a\xC3") == 2);
    CHECK(spike_str_length("\xE2\x82" "x") == 3);
}

static void test_case(void) {
    CHECK_STR(spike_str_upper("ação"), "AÇÃO");
    CHECK_STR(spike_str_lower("ÁRVORE"), "árvore");
    CHECK_STR(spike_str_upper("àáâãäçéêíóôõúü"), "ÀÁÂÃÄÇÉÊÍÓÔÕÚÜ");
    CHECK_STR(spike_str_lower("ÀÁÂÃÄÇÉÊÍÓÔÕÚÜ"), "àáâãäçéêíóôõúü");
    CHECK_STR(spike_str_upper("x÷y ß"), "X÷Y ß");
    CHECK_STR(spike_str_lower("A×B"), "a×b");
    CHECK_STR(spike_str_upper(""), "");
    CHECK_STR(spike_str_upper("\xF0\x9F\x98\x80" "a\xFF"), "\xF0\x9F\x98\x80" "A\xFF");
}

static void test_slice_and_find(void) {
    CHECK_STR(spike_str_slice("ação", 2, 2, 1), "çã");
    CHECK_STR(spike_str_slice("ação", 1, 10, 1), "ação");
    CHECK_STR(spike_str_slice("ação", 5, 1, 1), "");
    CHECK_STR(spike_str_slice("abc", 2, 0, 1), "");
    CHECK_STR(spike_str_slice("abc", 2, -1, 1), "");
    CHECK_STR(spike_str_slice("\xF0\x9F\x98\x80" "x", 1, 1, 1), "\xF0\x9F\x98\x80");
    CHECK(spike_str_find("ç", "ação") == 2);
    CHECK(spike_str_find("ão", "ação") == 3);
    CHECK(spike_str_find("x", "abc") == 0);
    CHECK(spike_str_find("", "abc") == 0);
    CHECK(spike_str_find("b", "\xF0\x9F\x98\x80" "b") == 2);
}

static void test_conversions(void) {
    CHECK_STR(spike_int_to_str(-42), "-42");
    CHECK_STR(spike_real_to_str(2.5), "2.5");
    CHECK_STR(spike_real_to_str(3.0), "3");
    CHECK_STR(spike_bool_to_str(1), "verdadeiro");
    CHECK_STR(spike_bool_to_str(0), "falso");
    CHECK(spike_str_to_int(" 42 ", 1) == 42);
    CHECK(spike_str_to_int("-7", 1) == -7);
    CHECK(spike_str_to_real("3,5", 1) == 3.5);
    CHECK(spike_str_to_real("10", 1) == 10.0);
    CHECK(spike_str_is_number("12") && spike_str_is_number(" 1.5 ") && spike_str_is_number("2,5"));
    CHECK(!spike_str_is_number("abc") && !spike_str_is_number("") && !spike_str_is_number("1.2.3"));
}

static void test_codes(void) {
    CHECK(spike_ord("A", 1) == 65);
    CHECK(spike_ord("ç", 1) == 231);
    CHECK(spike_ord("€", 1) == 8364);
    CHECK(spike_ord("\xF0\x9F\x98\x80", 1) == 128512);
    CHECK(spike_ord("\xFF", 1) == 255);
    CHECK_STR(spike_chr(65, 1), "A");
    CHECK_STR(spike_chr(231, 1), "ç");
    CHECK_STR(spike_chr(8364, 1), "€");
    CHECK_STR(spike_chr(128512, 1), "\xF0\x9F\x98\x80");
    CHECK(spike_ord(spike_chr(1114111, 1), 1) == 1114111);
}

int main(void) {
    spike_init();
    test_length();
    test_case();
    test_slice_and_find();
    test_conversions();
    test_codes();
    if (failures == 0) {
        printf("string tests passed\n");
        return 0;
    }
    fprintf(stderr, "%d string check(s) failed\n", failures);
    return 1;
}
