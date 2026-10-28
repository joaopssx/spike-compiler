#include "spike/explain.hpp"

#include "spike/colors.hpp"
#include "spike/error_catalog.hpp"

#include <ostream>
#include <string>
#include <string_view>

namespace spike {

namespace {

constexpr int kExitUsage = 2;

std::string text(std::string_view view) {
    return std::string(view);
}

int code_number(std::string_view code) {
    return std::stoi(std::string(code.substr(1)));
}

void write_indented(std::ostream& out, std::string_view block) {
    std::size_t start = 0;
    while (start <= block.size()) {
        std::size_t end = block.find('\n', start);
        if (end == std::string_view::npos) end = block.size();
        out << "    " << block.substr(start, end - start) << "\n";
        start = end + 1;
    }
}

void write_part(std::ostream& out, const std::string& label, std::string_view body) {
    out << bold(label, Stream::Out);
    if (body.find('\n') == std::string_view::npos) {
        out << " " << body << "\n\n";
        return;
    }
    out << "\n";
    write_indented(out, body);
    out << "\n";
}

void write_entry(std::ostream& out, const ErrorEntry& entry) {
    out << caramel(bold(text(entry.code) + " — " + text(entry.title), Stream::Out), Stream::Out)
        << "\n\n";
    write_part(out, "Mensagem:", entry.message);
    write_part(out, "Quando acontece:", entry.when);
    write_part(out, "Como corrigir:", entry.fix);
    out << bold("Exemplo:", Stream::Out) << "\n";
    write_indented(out, entry.example);
}

std::string_view first_line(std::string_view value) {
    return value.substr(0, value.find('\n'));
}

void write_list(std::ostream& out) {
    out << bold("códigos de erro do Spike", Stream::Out) << " "
        << gray("(spike --explain <código> mostra a explicação)", Stream::Out) << "\n";
    for (const ErrorEntry& entry : all_errors()) {
        out << "  " << cyan(text(entry.code), Stream::Out) << "  " << first_line(entry.message)
            << "\n";
    }
}

} // namespace

std::string summarize_codes() {
    std::string out;
    const auto& entries = all_errors();
    std::size_t i = 0;
    while (i < entries.size()) {
        std::size_t last = i;
        while (last + 1 < entries.size() && entries[last + 1].code[0] == entries[i].code[0] &&
               code_number(entries[last + 1].code) == code_number(entries[last].code) + 1) {
            ++last;
        }
        if (!out.empty()) out += ", ";
        out += text(entries[i].code);
        if (last > i) out += "-" + text(entries[last].code);
        i = last + 1;
    }
    return out;
}

int run_explain(const std::string& code, std::ostream& out, std::ostream& err) {
    if (code.empty()) {
        write_list(out);
        return 0;
    }
    const ErrorEntry* entry = find_error(code);
    if (entry == nullptr) {
        err << red(bold("erro:")) << " não conheço o código " << code
            << "; códigos válidos: " << summarize_codes() << "\n"
            << gray("use 'spike --explain' para ver a lista com as mensagens.") << "\n";
        return kExitUsage;
    }
    write_entry(out, *entry);
    return 0;
}

} // namespace spike
