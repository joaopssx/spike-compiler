#include "spike/process.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct Block {
    std::string file;
    int line = 0;
    std::string info;
    std::string text;
};

struct Settings {
    std::string spike;
    fs::path root;
    fs::path work;
    bool run = true;
};

std::string read_text(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string text = buffer.str();
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    return text;
}

void write_text(const fs::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << text;
}

std::vector<Block> fenced_blocks(const fs::path& path, const std::string& shown) {
    std::vector<Block> blocks;
    std::istringstream in(read_text(path));
    std::string line;
    int number = 0;
    Block* open = nullptr;
    while (std::getline(in, line)) {
        ++number;
        if (line.rfind("```", 0) != 0) {
            if (open) open->text += line + "\n";
            continue;
        }
        if (open) {
            open = nullptr;
            continue;
        }
        blocks.push_back(Block{shown, number, line.substr(3), ""});
        open = &blocks.back();
    }
    return blocks;
}

std::string trim_end(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == ' ')) text.pop_back();
    return text;
}

std::string first_word(const std::string& info) {
    return info.substr(0, info.find(' '));
}

spike::ProcessResult run(const std::vector<std::string>& args, const std::string& input = "") {
    spike::ProcessOptions options;
    options.args = args;
    options.capture_stdout = true;
    options.capture_stderr = true;
    if (!input.empty()) options.stdin_file = input;
    return spike::run_process(options);
}

class Checker {
public:
    explicit Checker(Settings settings) : settings_(std::move(settings)) {}

    int failures() const { return failures_; }
    int passed() const { return passed_; }
    int skipped() const { return skipped_; }

    void check_file(const std::vector<Block>& blocks) {
        for (std::size_t i = 0; i < blocks.size(); ++i) {
            const std::string kind = first_word(blocks[i].info);
            if (kind == "portugol") {
                check_compiles(blocks[i]);
            } else if (kind == "portugol-error") {
                check_error(blocks[i]);
            } else if (kind == "portugol-run") {
                const Block* input = nullptr;
                const Block* output = nullptr;
                std::size_t next = i + 1;
                if (next < blocks.size() && first_word(blocks[next].info) == "input") {
                    input = &blocks[next++];
                }
                if (next < blocks.size() && first_word(blocks[next].info) == "output") {
                    output = &blocks[next];
                }
                check_runs(blocks[i], input, output);
            }
        }
    }

private:
    Settings settings_;
    int counter_ = 0;
    int failures_ = 0;
    int passed_ = 0;
    int skipped_ = 0;

    void fail(const Block& block, const std::string& why, const std::string& detail) {
        ++failures_;
        std::cerr << block.file << ":" << block.line << ": " << why << "\n" << detail << "\n";
    }

    fs::path source_for(const Block& block) {
        const fs::path path = settings_.work / ("doc_" + std::to_string(++counter_) + ".por");
        write_text(path, block.text);
        return path;
    }

    void check_compiles(const Block& block) {
        const spike::ProcessResult result = run({settings_.spike, "check", source_for(block).u8string()});
        if (result.exit_code != 0) return fail(block, "example does not compile", result.out + result.err);
        ++passed_;
    }

    void check_error(const Block& block) {
        const std::string code = block.info.size() > 15 ? block.info.substr(15) : "";
        const spike::ProcessResult result = run({settings_.spike, "check", source_for(block).u8string()});
        const std::string all = result.out + result.err;
        if (code.empty()) return fail(block, "portugol-error needs a code, as in 'portugol-error E700'", "");
        if (result.exit_code == 0 || all.find("[" + code + "]") == std::string::npos) {
            return fail(block, "expected error " + code, all);
        }
        ++passed_;
    }

    void check_runs(const Block& block, const Block* input, const Block* output) {
        const fs::path source = source_for(block);
        if (!settings_.run) {
            ++skipped_;
            return check_compiles(block);
        }
        const fs::path exe = settings_.work / ("doc_" + std::to_string(counter_) + ".exe");
        const spike::ProcessResult built =
            run({settings_.spike, source.u8string(), "-o", exe.u8string()});
        if (built.exit_code != 0) return fail(block, "example does not compile", built.out + built.err);
        std::string input_path;
        if (input) {
            input_path = (settings_.work / ("doc_" + std::to_string(counter_) + ".in")).u8string();
            write_text(fs::u8path(input_path), input->text);
        }
        const spike::ProcessResult ran = run({exe.u8string()}, input_path);
        if (!ran.launched || ran.exit_code != 0) {
            return fail(block, "example did not run (exit " + std::to_string(ran.exit_code) + ")",
                        ran.out + ran.err + ran.launch_error);
        }
        std::string out = ran.out;
        out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());
        if (output && trim_end(out) != trim_end(output->text)) {
            return fail(block, "output differs from the ```output block",
                        "expected:\n" + output->text + "got:\n" + out);
        }
        ++passed_;
    }
};

std::vector<fs::path> markdown_files(const fs::path& root) {
    std::vector<fs::path> files = {root / "README.md"};
    for (const auto& entry : fs::recursive_directory_iterator(root / "docs")) {
        if (entry.is_regular_file() && entry.path().extension() == ".md") files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    return files;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: spike_doctest <spike> <repository> <work folder> [--no-run]\n";
        return 2;
    }
    Settings settings{argv[1], fs::u8path(argv[2]), fs::u8path(argv[3]),
                      !(argc > 4 && std::string(argv[4]) == "--no-run")};
    fs::create_directories(settings.work);
    Checker checker(settings);
    for (const fs::path& file : markdown_files(settings.root)) {
        const std::string shown = file.lexically_relative(settings.root).generic_u8string();
        checker.check_file(fenced_blocks(file, shown));
    }
    std::cout << "docs examples: " << checker.passed() << " passed, " << checker.failures()
              << " failed";
    if (checker.skipped() > 0) std::cout << " (" << checker.skipped() << " only checked, no C compiler)";
    std::cout << "\n";
    return checker.failures() == 0 ? 0 : 1;
}
