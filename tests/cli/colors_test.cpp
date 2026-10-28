#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cli.hpp"
#include "spike/colors.hpp"

#include <string>
#include <vector>

namespace {

using spike::ColorMode;

spike::CliArgs parse(const std::vector<std::string>& argv_in) {
    std::vector<char*> argv;
    for (const auto& s : argv_in) argv.push_back(const_cast<char*>(s.c_str()));
    return spike::parse_args(static_cast<int>(argv.size()), argv.data());
}

void test_auto_follows_the_terminal() {
    SPIKE_EXPECT(spike::should_color(ColorMode::Auto, true, nullptr, nullptr));
    SPIKE_EXPECT(!spike::should_color(ColorMode::Auto, false, nullptr, nullptr));
}

void test_no_color_turns_auto_off() {
    SPIKE_EXPECT(!spike::should_color(ColorMode::Auto, true, "1", nullptr));
    SPIKE_EXPECT(spike::should_color(ColorMode::Auto, true, "", nullptr));
}

void test_force_color_turns_auto_on() {
    SPIKE_EXPECT(spike::should_color(ColorMode::Auto, false, nullptr, "1"));
    SPIKE_EXPECT(!spike::should_color(ColorMode::Auto, false, nullptr, "0"));
    SPIKE_EXPECT(!spike::should_color(ColorMode::Auto, false, "1", "1"));
}

void test_explicit_modes_ignore_environment() {
    SPIKE_EXPECT(spike::should_color(ColorMode::Always, false, "1", nullptr));
    SPIKE_EXPECT(!spike::should_color(ColorMode::Never, true, nullptr, "1"));
}

void test_paint_respects_the_mode() {
    const ColorMode saved = spike::color_mode();
    spike::set_color_mode(ColorMode::Never);
    SPIKE_EXPECT(spike::red("x") == "x");
    SPIKE_EXPECT(spike::bold("y", spike::Stream::Out) == "y");
    spike::set_color_mode(ColorMode::Always);
    SPIKE_EXPECT(spike::red("x") == std::string(spike::Color::RED) + "x" + spike::Color::RESET);
    SPIKE_EXPECT(spike::enabled(spike::Stream::Out));
    spike::set_color_mode(saved);
}

void test_cli_color_flag() {
    SPIKE_EXPECT(parse({"spike", "a.por"}).color == ColorMode::Auto);
    SPIKE_EXPECT(parse({"spike", "a.por", "--color=always"}).color == ColorMode::Always);
    SPIKE_EXPECT(parse({"spike", "--color=never", "check", "a.por"}).color == ColorMode::Never);
    const auto args = parse({"spike", "check", "a.por", "--color=auto"});
    SPIKE_EXPECT(!args.parse_error);
    SPIKE_EXPECT(args.command == spike::Command::CHECK);
    SPIKE_EXPECT(args.input_file == "a.por");
}

void test_cli_color_flag_rejects_bad_value() {
    const auto args = parse({"spike", "a.por", "--color=sim"});
    SPIKE_EXPECT(args.parse_error);
    SPIKE_EXPECT(args.parse_error_msg.find("auto, always ou never") != std::string::npos);
    SPIKE_EXPECT(parse({"spike", "a.por", "--color"}).parse_error);
}

} // namespace

namespace spike_tests {

void run_colors_tests() {
    test_auto_follows_the_terminal();
    test_no_color_turns_auto_off();
    test_force_color_turns_auto_on();
    test_explicit_modes_ignore_environment();
    test_paint_respects_the_mode();
    test_cli_color_flag();
    test_cli_color_flag_rejects_bad_value();
}

} // namespace spike_tests
