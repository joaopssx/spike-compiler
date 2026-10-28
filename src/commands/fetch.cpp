#include "spike/cc_driver.hpp"
#include "spike/colors.hpp"
#include "spike/commands.hpp"
#include "spike/process.hpp"
#include "spike/temp_dir.hpp"

#include <cstddef>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/utsname.h>
#endif

namespace spike {

namespace {

constexpr int kExitOk = 0;
constexpr int kExitMissing = 3;
constexpr const char* kIssuesUrl = "https://github.com/joaopssx/spike-compiler/issues";

const std::vector<std::string>& dog() {
    static const std::vector<std::string> lines = {
        "    / \\__      ",
        "   (    @\\___  ",
        "   /         O ",
        "  /   (_____/  ",
        " /_____/   U   ",
    };
    return lines;
}

std::string label(const std::string& name) {
    return cyan(name + ":", Stream::Out) + " ";
}

struct CompilerCheck {
    bool ok = false;
    std::string where;
    std::string version;
    std::string hint;
};

CompilerCheck check_compiler() {
    CompilerCheck check;
    const std::optional<CompilerInfo> compiler = find_c_compiler();
    if (!compiler) {
        const DriverError error = no_compiler_error();
        check.where = red("não encontrado", Stream::Out) + " (" + error.message + ")";
        check.hint = error.hint;
        return check;
    }
    check.where = compiler->kind + " em " + compiler->path;
    ProcessOptions options;
    options.args = {compiler->path, "--version"};
    options.capture_stdout = true;
    options.capture_stderr = true;
    const ProcessResult result = run_process(options);
    if (!result.launched || result.exit_code != 0) {
        check.version = red("'" + compiler->path + " --version' falhou", Stream::Out);
        check.hint = "o compilador foi achado, mas não roda. Reinstale-o ou aponte SPIKE_CC "
                     "para outro.";
        return check;
    }
    check.version = first_line(result.out.empty() ? result.err : result.out);
    check.ok = true;
    return check;
}

struct TempCheck {
    bool ok = false;
    std::string text;
};

TempCheck check_temp_folder() {
    TempDir temp;
    if (!temp.ok()) return TempCheck{false, red(temp.error(), Stream::Out)};
    std::string error;
    if (!write_text_file(temp.path() / "probe.txt", "spike", error)) {
        return TempCheck{false, red("sem permissão de escrita: " + error, Stream::Out)};
    }
    return TempCheck{true, "gravável (" + temp.path().parent_path().u8string() + ")"};
}

std::string spike_cc_text() {
    const std::optional<std::string> forced = get_env("SPIKE_CC");
    if (!forced || forced->empty()) return "não definido (o Spike procura no PATH)";
    return "'" + *forced + "'";
}

} // namespace

std::string os_description() {
#ifdef _WIN32
    return "Windows";
#else
    struct utsname info;
    if (uname(&info) == 0) return std::string(info.sysname) + " " + info.release;
#if defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "desconhecido";
#endif
#endif
}

std::string arch_name() {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#else
    return "desconhecida";
#endif
}

std::string first_line(const std::string& text) {
    std::string line = text.substr(0, text.find('\n'));
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

int run_fetch(std::ostream& out) {
    const CompilerCheck compiler = check_compiler();
    const TempCheck temp = check_temp_folder();

    std::vector<std::string> info = {
        bold(caramel(std::string("Spike ") + SPIKE_VERSION, Stream::Out), Stream::Out),
        label("sistema") + os_description() + " (" + arch_name() + ")",
        label("compilador C") + compiler.where,
    };
    if (!compiler.version.empty()) info.push_back(label("versão do C") + compiler.version);
    info.push_back(label("SPIKE_CC") + spike_cc_text());
    info.push_back(label("pasta temporária") + temp.text);

    const std::vector<std::string>& art = dog();
    const std::size_t rows = art.size() > info.size() ? art.size() : info.size();
    const std::string blank(art.front().size(), ' ');
    for (std::size_t i = 0; i < rows; ++i) {
        const std::string left = i < art.size() ? caramel(art[i], Stream::Out) : blank;
        out << left << (i < info.size() ? info[i] : "") << "\n";
    }
    out << "\n";

    const bool ready = compiler.ok && temp.ok;
    if (ready) {
        out << green("tudo certo:", Stream::Out) << " o Spike consegue gerar executáveis.\n";
    } else {
        if (!compiler.ok) out << red("falta:", Stream::Out) << " " << compiler.hint << "\n";
        if (!temp.ok) {
            out << red("falta:", Stream::Out)
                << " uma pasta temporária gravável (confira TEMP/TMPDIR).\n";
        }
    }
    out << gray(std::string("pedindo ajuda? cole esta saída na issue: ") + kIssuesUrl,
                Stream::Out)
        << std::endl;
    return ready ? kExitOk : kExitMissing;
}

} // namespace spike
