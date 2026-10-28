#include "codegen/codegen_internal.hpp"

#include <string>
#include <vector>

namespace spike::codegen_detail {

namespace {

std::vector<std::string> source_lines(const std::string& source) {
    std::vector<std::string> lines;
    std::string current;
    for (char c : source) {
        if (c != '\n') {
            current += c;
            continue;
        }
        if (!current.empty() && current.back() == '\r') current.pop_back();
        lines.push_back(current);
        current.clear();
    }
    if (!current.empty() && current.back() == '\r') current.pop_back();
    if (!current.empty() || lines.empty()) lines.push_back(current);
    return lines;
}

} // namespace

void Generator::emit_source_lines() {
    out_.line("static const char* const spike_source_name = " + string_literal(source_name_) +
              ";");
    out_.line("static const char* const spike_source_lines[] = {");
    out_.indent();
    for (const std::string& text : source_lines(options_.source)) {
        out_.line(string_literal(text) + ",");
    }
    out_.dedent();
    out_.line("};");
    out_.blank();
}

void Generator::emit_source_setup() {
    out_.line("spike_set_source(spike_source_name, spike_source_lines, " +
              std::to_string(source_lines(options_.source).size()) + ");");
}

void Generator::mark_source_line(int line) {
    if (!options_.emit_line_directives || line <= 0) return;
    out_.directive("#line " + std::to_string(line) + " " + string_literal(source_name_));
    in_source_lines_ = true;
}

void Generator::mark_generated_code() {
    if (!in_source_lines_) return;
    out_.directive("#line " + std::to_string(out_.next_line() + 1) + " " +
                   string_literal(options_.c_file_name));
    in_source_lines_ = false;
}

void Generator::emit_user_block(const StmtList& stmts) {
    emit_block(stmts);
    mark_generated_code();
}

} // namespace spike::codegen_detail
