#include "spike/cc_driver.hpp"
#include "spike/colors.hpp"
#include "spike/process.hpp"
#include "spike/setup.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace spike {

namespace {

namespace fs = std::filesystem;

constexpr int kExitOk = 0;
constexpr int kExitMissing = 3;

std::string megabytes(std::uint64_t bytes) {
    char text[32];
    std::snprintf(text, sizeof text, "%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
    return text;
}

bool confirm(const std::string& question, bool assume_yes) {
    if (assume_yes) return true;
    std::cerr << question << " [s/N] " << std::flush;
    std::string answer;
    if (!std::getline(std::cin, answer)) return false;
    return answer == "s" || answer == "S" || answer == "sim" || answer == "y" || answer == "yes";
}

void print_error(const SetupError& error) {
    std::cerr << red(bold("erro[" + error.code + "]:")) << " " << error.message << "\n";
    if (!error.hint.empty()) std::cerr << "     " << green("= dica:") << " " << error.hint << "\n";
}

std::string install_instructions() {
#if defined(_WIN32)
    return "rode: spike setup";
#elif defined(__APPLE__)
    return "rode no Terminal: xcode-select --install";
#else
    std::ifstream in("/etc/os-release");
    std::stringstream text;
    text << in.rdbuf();
    const LinuxFamily family = detect_linux_family(text.str());
    const std::string command = linux_install_command(family);
    if (family == LinuxFamily::Unknown) return "instale o gcc com o gerenciador da sua distribuição:\n" + command;
    return "rode no terminal: " + command;
#endif
}

int report_state(const std::optional<CompilerInfo>& compiler) {
    if (compiler) {
        std::cerr << green(bold("ok:")) << " o compilador C já está instalado: " << compiler->path
                  << " (" << compiler->kind << ")\n";
        return kExitOk;
    }
    const std::optional<std::string> forced = get_env("SPIKE_CC");
    if (forced && !forced->empty()) {
        std::cerr << red(bold("nenhum compilador C encontrado:")) << " SPIKE_CC aponta para '"
                  << *forced << "', que não existe.\ncorrija o caminho ou apague a variável SPIKE_CC.\n";
        return kExitMissing;
    }
    std::cerr << red(bold("nenhum compilador C encontrado.")) << "\n" << install_instructions()
              << "\n";
    return kExitMissing;
}

#ifdef _WIN32

int remove_installed(const CliArgs& args) {
    const std::optional<fs::path> local = local_app_data_dir();
    if (!local) {
        std::cerr << "a variável LOCALAPPDATA não existe; não há o que apagar.\n";
        return kExitOk;
    }
    const SetupPaths paths = setup_paths(*local, pinned_toolchain());
    const fs::path target = paths.base / "toolchain";
    std::error_code ec;
    if (!fs::exists(target, ec)) {
        std::cerr << "não há compilador instalado pelo Spike em '" << target.u8string() << "'.\n";
        return kExitOk;
    }
    if (!confirm("Apagar a pasta '" + target.u8string() + "'?", args.setup_yes)) {
        std::cerr << "nada foi apagado.\n";
        return kExitOk;
    }
    std::string error;
    if (!remove_toolchain(paths, target, error)) {
        std::cerr << red(bold("erro:")) << " " << error << "\n";
        return kExitMissing;
    }
    std::cerr << green(bold("ok:")) << " pasta '" << target.u8string() << "' apagada.\n";
    return kExitOk;
}

int install(const CliArgs& args) {
    const std::optional<fs::path> local = local_app_data_dir();
    if (!local) {
        print_error({"E904", "a variável LOCALAPPDATA não existe", "o Windows sempre a define; "
                     "confira se o Spike não está rodando num ambiente modificado."});
        return kExitMissing;
    }
    const ToolchainPin& pin = pinned_toolchain();
    const SetupPaths paths = setup_paths(*local, pin);
    std::cerr << "O Spike vai baixar " << pin.name << " " << pin.version << " (GCC para Windows, "
              << megabytes(pin.size) << ")\n  de " << pin.url << "\n  para "
              << paths.toolchain_dir.u8string() << "\n";
    if (!confirm("Continuar?", args.setup_yes)) {
        std::cerr << "nada foi baixado. Para não perguntar, use 'spike setup --yes'.\n";
        return kExitOk;
    }
    InstallTools tools = system_install_tools();
    tools.progress = [](std::uint64_t done, std::uint64_t total) {
        std::cerr << "\rbaixando: " << megabytes(done);
        if (total > 0) std::cerr << " de " << megabytes(total) << " (" << done * 100 / total << "%)";
        std::cerr << std::flush;
    };
    const InstallResult result = install_toolchain(pin, paths, tools);
    std::cerr << "\n";
    if (!result.ok) {
        print_error(result.error);
        return kExitMissing;
    }
    std::cerr << green(bold("ok:")) << " " << result.gcc_version << "\n  instalado em "
              << result.gcc_path << "\n  o Spike vai usar esse compilador a partir de agora.\n";
    return kExitOk;
}

#endif

} // namespace

int run_setup(const CliArgs& args) {
    const std::optional<CompilerInfo> compiler = find_c_compiler();
    if (args.setup_check) return report_state(compiler);
#ifdef _WIN32
    if (args.setup_remove) return remove_installed(args);
    if (compiler && !args.setup_force) return report_state(compiler);
    return install(args);
#else
    if (args.setup_remove) {
        std::cerr << "o Spike só instala o compilador sozinho no Windows; não há o que apagar.\n";
        return kExitOk;
    }
    return report_state(compiler);
#endif
}

} // namespace spike
