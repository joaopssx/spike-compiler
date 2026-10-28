#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "io/text_encoding.hpp"
#include "spike/ast_printer.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace spike::text_encoding;

fs::path temp_folder() {
    const fs::path folder = fs::u8path(std::string(SPIKE_BINARY_DIR) + "/encoding_tests");
    fs::create_directories(folder);
    return folder;
}

spike::ReadResult read_bytes(const std::string& name, const std::string& bytes) {
    const fs::path path = temp_folder() / fs::u8path(name);
    {
        std::ofstream out(path, std::ios::binary);
        out << bytes;
    }
    return spike::read_file(path.u8string());
}

void test_newlines_and_bom() {
    SPIKE_EXPECT(normalize_newlines("a\r\nb\rc\n\r\n") == "a\nb\nc\n\n");
    std::string text = "\xEF\xBB\xBFinicio";
    SPIKE_EXPECT(strip_utf8_bom(text) && text == "inicio");
    SPIKE_EXPECT(!strip_utf8_bom(text) && text == "inicio");
    const auto crlf = read_bytes("crlf.por", "\xEF\xBB\xBF" "a\r\nb\rc");
    SPIKE_EXPECT(crlf.ok && crlf.content == "a\nb\nc");
    SPIKE_EXPECT(!crlf.encoding.converted_from_cp1252);
}

void test_cp1252_table() {
    SPIKE_EXPECT(cp1252_to_utf8("a\xE7\xE3o") == "aç\xC3\xA3o");
    SPIKE_EXPECT(cp1252_to_utf8("\x80\x93\x9F") == "€“Ÿ");
    SPIKE_EXPECT(cp1252_to_utf8("\x81") == "\xEF\xBF\xBD");
    SPIKE_EXPECT(cp1252_to_utf8("\xFF") == "ÿ");
}

void test_plausible_cp1252() {
    const auto converted = plausible_cp1252_to_utf8("escreval(\"a\xE7\xE3o\")\n");
    SPIKE_EXPECT(converted && *converted == "escreval(\"ação\")\n");
    SPIKE_EXPECT(!plausible_cp1252_to_utf8("a\x01\xE7"));
    SPIKE_EXPECT(!plausible_cp1252_to_utf8("a\x81"));
    SPIKE_EXPECT(!plausible_cp1252_to_utf8("\xC3\xA7 e \xE7"));
}

void test_read_cp1252_file() {
    const auto file = read_bytes("ansi.por", "algoritmo \"t\"\r\ninicio\r\n  escreval(\"S\xE3o\")\r\n");
    SPIKE_EXPECT(file.ok && file.encoding.converted_from_cp1252);
    SPIKE_EXPECT(file.content == "algoritmo \"t\"\ninicio\n  escreval(\"São\")\n");
    SPIKE_EXPECT(file.encoding.line == 3 && file.encoding.col == 14);
    const auto broken = read_bytes("broken.por", "inicio\r\n\x01\xFF");
    SPIKE_EXPECT(broken.ok && !broken.encoding.converted_from_cp1252);
    SPIKE_EXPECT(broken.content == "inicio\n\x01\xFF");
}

void test_accented_path() {
    const auto file = read_bytes("pasta ação é.por", "inicio\n");
    SPIKE_EXPECT(file.ok && file.content == "inicio\n");
}

struct FrontEnd {
    std::vector<spike::Token> tokens;
    std::string ast;
    std::string diagnostics;
};

FrontEnd front_end(const std::string& source) {
    FrontEnd out;
    spike::Lexer lexer(source, "prog.por");
    out.tokens = lexer.tokenize();
    spike::Parser parser(out.tokens, "prog.por", source);
    out.ast = spike::print_ast(parser.parse());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    parser.diagnostics().print_all();
    std::cerr.rdbuf(old);
    out.diagnostics = sink.str();
    return out;
}

void test_crlf_and_lf_are_identical() {
    const std::string lf = "algoritmo \"t\"\nvar\n  x : inteiro\ninicio\n  x <- 1 +\n"
                           "  escreval(\"ação\", x)\nfimalgoritmo\n";
    std::string crlf;
    for (char c : lf) crlf += c == '\n' ? std::string("\r\n") : std::string(1, c);
    const FrontEnd a = front_end(read_bytes("lf.por", lf).content);
    const FrontEnd b = front_end(read_bytes("crlf2.por", crlf).content);
    SPIKE_EXPECT(a.tokens.size() == b.tokens.size());
    for (std::size_t i = 0; i < a.tokens.size() && i < b.tokens.size(); ++i) {
        SPIKE_EXPECT(a.tokens[i].type == b.tokens[i].type && a.tokens[i].lexeme == b.tokens[i].lexeme &&
                     a.tokens[i].line == b.tokens[i].line && a.tokens[i].col == b.tokens[i].col);
    }
    SPIKE_EXPECT(a.ast == b.ast);
    SPIKE_EXPECT(!a.diagnostics.empty() && a.diagnostics == b.diagnostics);
}

} // namespace

namespace spike_tests {

void run_file_encoding_tests() {
    test_newlines_and_bom();
    test_cp1252_table();
    test_plausible_cp1252();
    test_read_cp1252_file();
    test_accented_path();
    test_crlf_and_lf_are_identical();
}

} // namespace spike_tests
