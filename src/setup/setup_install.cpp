#include "spike/process.hpp"
#include "spike/setup.hpp"
#include "spike/sha256.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace spike {

namespace fs = std::filesystem;

std::string winhttp_download(const std::string& url, const fs::path& destination,
                             const DownloadProgress& progress);

namespace {

InstallResult failure(const std::string& code, const std::string& message,
                      const std::string& hint) {
    InstallResult result;
    result.error = SetupError{code, message, hint};
    return result;
}

void remove_quietly(const fs::path& path) {
    std::error_code ec;
    fs::remove(path, ec);
}

std::string extract_with_sfx(const fs::path& archive, const fs::path& folder) {
    ProcessOptions options;
    options.args = {archive.u8string(), "-y", "-o" + folder.u8string()};
    options.capture_stdout = true;
    options.capture_stderr = true;
    const ProcessResult result = run_process(options);
    if (!result.launched) return result.launch_error;
    if (result.exit_code != 0) return "o extrator terminou com código " + std::to_string(result.exit_code);
    return "";
}

std::optional<std::string> probe_gcc(const std::string& gcc) {
    ProcessOptions options;
    options.args = {gcc, "--version"};
    options.capture_stdout = true;
    options.capture_stderr = true;
    const ProcessResult result = run_process(options);
    if (!result.launched || result.exit_code != 0) return std::nullopt;
    const std::string first = result.out.substr(0, result.out.find('\n'));
    return first.empty() ? std::string("gcc") : first;
}

} // namespace

InstallResult install_toolchain(const ToolchainPin& pin, const SetupPaths& paths,
                                const InstallTools& tools) {
    std::error_code ec;
    fs::create_directories(paths.toolchain_dir, ec);
    if (ec) {
        return failure("E904", "não consegui criar a pasta '" + paths.toolchain_dir.u8string() + "'",
                       "confira se há espaço em disco e permissão de escrita.");
    }
    const fs::path part = paths.toolchain_dir / fs::u8path(pin.archive_name + ".part");
    const fs::path archive = paths.toolchain_dir / fs::u8path(pin.archive_name);
    remove_quietly(part);

    const std::string download_error = tools.download(pin.url, part, tools.progress);
    if (!download_error.empty()) {
        remove_quietly(part);
        return failure("E906",
                       "não consegui baixar: " + download_error +
                           "; verifique a conexão, proxy ou antivírus",
                       "rode 'spike setup' de novo quando a internet voltar. Nada foi instalado.");
    }

    const std::uint64_t size = fs::file_size(part, ec);
    const std::optional<std::string> hash = sha256_file(part.u8string());
    if (ec || size != pin.size || !hash || *hash != pin.sha256) {
        remove_quietly(part);
        return failure("E905",
                       "o arquivo baixado não confere com o SHA-256 esperado e foi apagado",
                       "nada foi executado. Esperado " + pin.sha256 + " (" +
                           std::to_string(pin.size) + " bytes), recebido " +
                           (hash ? *hash : std::string("ilegível")) + " (" + std::to_string(size) +
                           " bytes). Tente de novo; se repetir, um proxy ou antivírus pode estar "
                           "alterando o download.");
    }
    fs::rename(part, archive, ec);
    if (ec) {
        remove_quietly(part);
        return failure("E904", "não consegui gravar '" + archive.u8string() + "'", ec.message());
    }

    const std::string extract_error = tools.extract(archive, paths.toolchain_dir);
    remove_quietly(archive);
    if (!extract_error.empty()) {
        return failure("E907", "não consegui extrair o compilador: " + extract_error,
                       "rode 'spike setup --force' para tentar de novo.");
    }
    const fs::path gcc = paths.toolchain_dir / fs::u8path(pin.gcc_relative_path);
    if (!fs::is_regular_file(gcc, ec)) {
        return failure("E907", "o gcc.exe não apareceu em '" + gcc.u8string() + "' depois de extrair",
                       "rode 'spike setup --force' para tentar de novo.");
    }
    const std::optional<std::string> version = tools.probe(gcc.u8string());
    if (!version) {
        return failure("E907", "o gcc baixado não rodou ('gcc --version' falhou)",
                       "um antivírus pode ter bloqueado o compilador; libere a pasta '" +
                           paths.base.u8string() + "' e rode 'spike setup --force'.");
    }

    std::ofstream record(paths.record_file, std::ios::binary | std::ios::trunc);
    record << gcc.u8string() << "\n";
    if (!record) {
        return failure("E904", "não consegui gravar '" + paths.record_file.u8string() + "'",
                       "confira a permissão de escrita na pasta.");
    }
    InstallResult result;
    result.ok = true;
    result.gcc_path = gcc.u8string();
    result.gcc_version = *version;
    return result;
}

bool remove_toolchain(const SetupPaths& paths, const fs::path& target, std::string& error) {
    if (!is_inside(target, paths.base)) {
        error = "recusei apagar '" + target.u8string() + "': fica fora de '" +
                paths.base.u8string() + "'";
        return false;
    }
    std::error_code ec;
    fs::remove_all(target, ec);
    if (ec) {
        error = "não consegui apagar '" + target.u8string() + "': " + ec.message();
        return false;
    }
    fs::remove(paths.record_file, ec);
    return true;
}

InstallTools system_install_tools() {
    InstallTools tools;
    tools.download = winhttp_download;
    tools.extract = extract_with_sfx;
    tools.probe = probe_gcc;
    return tools;
}

} // namespace spike
