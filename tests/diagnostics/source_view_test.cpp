#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "diagnostics/source_view.hpp"
#include "diagnostics/utf8.hpp"

#include <string>

namespace {

namespace sv = spike::source_view;

void test_cells_count_characters_not_bytes() {
    SPIKE_EXPECT(sv::display_cells("Olá").size() == 3);
    SPIKE_EXPECT(sv::display_cells("ação").size() == 4);
    SPIKE_EXPECT(sv::display_cells("").empty());
}

void test_tab_becomes_four_spaces() {
    SPIKE_EXPECT(sv::display_cells("\tx").size() == 5);
    const sv::Window whole = sv::window_for("\tx", 0);
    SPIKE_EXPECT(sv::render("\tx", whole) == "    x");
    SPIKE_EXPECT(sv::render("a\tb", whole) == "a    b");
}

void test_invalid_byte_is_shown_as_question_mark() {
    const std::string line = std::string("a") + static_cast<char>(0xFF) + "b";
    SPIKE_EXPECT(sv::render(line, sv::window_for(line, 0)) == "a?b");
}

void test_caret_cell_walks_the_line() {
    SPIKE_EXPECT(sv::caret_cell("abc", 1) == 0);
    SPIKE_EXPECT(sv::caret_cell("abc", 3) == 2);
    SPIKE_EXPECT(sv::caret_cell("Olá @", 5) == 4);
    SPIKE_EXPECT(sv::caret_cell("\t@", 2) == 4);
    SPIKE_EXPECT(sv::caret_cell("\t\té@", 4) == 9);
}

void test_caret_cell_past_end_of_line() {
    SPIKE_EXPECT(sv::caret_cell("x <-", 5) == 4);
    SPIKE_EXPECT(sv::caret_cell("é", 3) == 2);
    SPIKE_EXPECT(sv::caret_cell("", 1) == 0);
}

void test_short_line_is_shown_whole() {
    const std::string line(sv::kLongLine, 'a');
    const sv::Window window = sv::window_for(line, 99);
    SPIKE_EXPECT(window.whole);
    SPIKE_EXPECT(sv::render(line, window) == line);
    SPIKE_EXPECT(sv::caret_offset(99, window) == 99);
}

void test_long_line_window_in_the_middle() {
    const std::string line(200, 'a');
    const sv::Window window = sv::window_for(line, 120);
    SPIKE_EXPECT(!window.whole);
    SPIKE_EXPECT(window.first == 80);
    SPIKE_EXPECT(window.last == 160);
    const std::string shown = sv::render(line, window);
    SPIKE_EXPECT(shown == "..." + std::string(80, 'a') + "...");
    SPIKE_EXPECT(sv::caret_offset(120, window) == 43);
}

void test_long_line_window_at_the_start() {
    const std::string line(150, 'a');
    const sv::Window window = sv::window_for(line, 5);
    SPIKE_EXPECT(window.first == 0);
    SPIKE_EXPECT(sv::render(line, window) == std::string(80, 'a') + "...");
    SPIKE_EXPECT(sv::caret_offset(5, window) == 5);
}

void test_long_line_window_at_the_end() {
    const std::string line(150, 'a');
    const sv::Window window = sv::window_for(line, 149);
    SPIKE_EXPECT(window.first == 70);
    SPIKE_EXPECT(window.last == 150);
    SPIKE_EXPECT(sv::render(line, window) == "..." + std::string(80, 'a'));
    SPIKE_EXPECT(sv::caret_offset(149, window) == 82);
}

void test_utf8_validation() {
    namespace u8 = spike::utf8;
    SPIKE_EXPECT(u8::first_invalid("Olá, ação, \xF0\x9F\x98\x80") == std::string::npos);
    SPIKE_EXPECT(u8::first_invalid(std::string("ab") + static_cast<char>(0xE9)) == 2);
    SPIKE_EXPECT(u8::first_invalid(std::string("\xC3")) == 0);
    SPIKE_EXPECT(u8::first_invalid(std::string("\xC0\x80")) == 0);
    SPIKE_EXPECT(u8::first_invalid(std::string("\xED\xA0\x80")) == 0);
    SPIKE_EXPECT(u8::first_invalid(std::string("\xF5\x80\x80\x80")) == 0);
}

void test_strip_accents() {
    namespace u8 = spike::utf8;
    SPIKE_EXPECT(u8::strip_accents("média") == "media");
    SPIKE_EXPECT(u8::strip_accents("Ação") == "Acao");
    SPIKE_EXPECT(u8::strip_accents("ÍNDICE") == "INDICE");
    SPIKE_EXPECT(u8::strip_accents("coração2") == "coracao2");
    SPIKE_EXPECT(u8::is_latin_letter(U'ç'));
    SPIKE_EXPECT(u8::is_latin_letter(U'ž'));
    SPIKE_EXPECT(!u8::is_latin_letter(U'×'));
    SPIKE_EXPECT(!u8::is_latin_letter(U'←'));
}

} // namespace

namespace spike_tests {

void run_source_view_tests() {
    test_cells_count_characters_not_bytes();
    test_tab_becomes_four_spaces();
    test_invalid_byte_is_shown_as_question_mark();
    test_caret_cell_walks_the_line();
    test_caret_cell_past_end_of_line();
    test_short_line_is_shown_whole();
    test_long_line_window_in_the_middle();
    test_long_line_window_at_the_start();
    test_long_line_window_at_the_end();
    test_utf8_validation();
    test_strip_accents();
}

} // namespace spike_tests
