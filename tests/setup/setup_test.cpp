#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cli.hpp"
#include "spike/setup.hpp"
#include "spike/sha256.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using spike::LinuxFamily;

const std::string kArchiveBytes = "gcc fake";

fs::path fresh_folder(const std::string& name) {
    const fs::path folder = fs::u8path(std::string(SPIKE_BINARY_DIR) + "/setup_tests/" + name);
    std::error_code ec;
    fs::remove_all(folder, ec);
    fs::create_directories(folder);
    return folder;
}

void write_file(const fs::path& path, const std::string& bytes) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << bytes;
}

void test_linux_families() {
    SPIKE_EXPECT(spike::detect_linux_family("NAME=\"Debian GNU/Linux\"\nID=debian\n") ==
                 LinuxFamily::Debian);
    SPIKE_EXPECT(spike::detect_linux_family("ID=ubuntu\nID_LIKE=debian\n") == LinuxFamily::Debian);
    SPIKE_EXPECT(spike::detect_linux_family("ID=fedora\r\nVERSION_ID=41\r\n") == LinuxFamily::Fedora);
    SPIKE_EXPECT(spike::detect_linux_family("ID=arch\n") == LinuxFamily::Arch);
    SPIKE_EXPECT(spike::detect_linux_family("ID=\"opensuse-tumbleweed\"\nID_LIKE=\"opensuse suse\"\n") ==
                 LinuxFamily::OpenSuse);
    SPIKE_EXPECT(spike::detect_linux_family("ID=alpine\n") == LinuxFamily::Alpine);
    SPIKE_EXPECT(spike::detect_linux_family("ID=nixos\n") == LinuxFamily::Unknown);
    SPIKE_EXPECT(spike::detect_linux_family("ID=pop\nID_LIKE=\"ubuntu debian\"\n") ==
                 LinuxFamily::Debian);
    SPIKE_EXPECT(spike::detect_linux_family("") == LinuxFamily::Unknown);
    SPIKE_EXPECT(spike::linux_install_command(LinuxFamily::Debian) ==
                 "sudo apt install build-essential");
    SPIKE_EXPECT(spike::linux_install_command(LinuxFamily::Fedora) == "sudo dnf install gcc");
    SPIKE_EXPECT(spike::linux_install_command(LinuxFamily::Arch) == "sudo pacman -S gcc");
    SPIKE_EXPECT(spike::linux_install_command(LinuxFamily::OpenSuse) == "sudo zypper install gcc");
    const std::string all = spike::linux_install_command(LinuxFamily::Unknown);
    SPIKE_EXPECT(all.find("apt") != std::string::npos && all.find("dnf") != std::string::npos &&
                 all.find("pacman") != std::string::npos && all.find("zypper") != std::string::npos);
}

void test_paths_and_pin() {
    const spike::ToolchainPin& pin = spike::pinned_toolchain();
    SPIKE_EXPECT(pin.url.rfind("https://github.com/skeeto/w64devkit/releases/download/", 0) == 0);
    SPIKE_EXPECT(pin.sha256.size() == 64 && pin.size > 0);
    const spike::SetupPaths paths = spike::setup_paths(fs::u8path("C:/Users/João/AppData/Local"), pin);
    SPIKE_EXPECT(paths.base == fs::u8path("C:/Users/João/AppData/Local/Spike"));
    SPIKE_EXPECT(paths.toolchain_dir == paths.base / "toolchain" / fs::u8path(pin.version));
    SPIKE_EXPECT(paths.record_file == paths.base / "toolchain.txt");
}

void test_is_inside() {
    const fs::path base = fresh_folder("inside") / "Spike";
    SPIKE_EXPECT(spike::is_inside(base / "toolchain", base));
    SPIKE_EXPECT(spike::is_inside(base / "toolchain" / "2.10.1", base));
    SPIKE_EXPECT(!spike::is_inside(base, base));
    SPIKE_EXPECT(!spike::is_inside(base / "..", base));
    SPIKE_EXPECT(!spike::is_inside(base / "toolchain" / ".." / "..", base));
    SPIKE_EXPECT(!spike::is_inside(base.parent_path() / "Spike2", base));
    SPIKE_EXPECT(!spike::is_inside(base.parent_path(), base));
}

void test_recorded_toolchain() {
    const fs::path folder = fresh_folder("record");
    const fs::path gcc = folder / "bin" / "gcc.exe";
    write_file(gcc, "x");
    write_file(folder / "toolchain.txt", gcc.u8string() + "\r\n");
    const auto found = spike::recorded_toolchain_gcc(folder / "toolchain.txt");
    SPIKE_EXPECT(found && *found == gcc.u8string());
    write_file(folder / "missing.txt", (folder / "nada.exe").u8string());
    SPIKE_EXPECT(!spike::recorded_toolchain_gcc(folder / "missing.txt"));
    SPIKE_EXPECT(!spike::recorded_toolchain_gcc(folder / "nao-existe.txt"));
}

spike::ToolchainPin fake_pin(const std::string& bytes) {
    spike::ToolchainPin pin;
    pin.name = "fake";
    pin.version = "1.0";
    pin.url = "https://example.invalid/fake.7z.exe";
    pin.size = kArchiveBytes.size();
    pin.sha256 = spike::sha256_hex(bytes);
    pin.archive_name = "fake.7z.exe";
    pin.gcc_relative_path = "kit/bin/gcc.exe";
    return pin;
}

struct Fakes {
    std::string served = kArchiveBytes;
    std::string download_error;
    std::string extract_error;
    bool extract_creates_gcc = true;
    bool gcc_runs = true;
    std::vector<std::string> calls;
};

spike::InstallTools fake_tools(Fakes& fakes) {
    spike::InstallTools tools;
    tools.download = [&fakes](const std::string&, const fs::path& to, const spike::DownloadProgress&) {
        fakes.calls.push_back("download");
        if (!fakes.download_error.empty()) return fakes.download_error;
        write_file(to, fakes.served);
        return std::string();
    };
    tools.extract = [&fakes](const fs::path& archive, const fs::path& folder) {
        fakes.calls.push_back("extract:" + archive.filename().u8string());
        if (fakes.extract_creates_gcc) write_file(folder / "kit" / "bin" / "gcc.exe", "binary");
        return fakes.extract_error;
    };
    tools.probe = [&fakes](const std::string&) -> std::optional<std::string> {
        fakes.calls.push_back("probe");
        if (!fakes.gcc_runs) return std::nullopt;
        return std::string("gcc (fake) 16.2.0");
    };
    return tools;
}

spike::InstallResult install_with(Fakes& fakes, const std::string& folder_name,
                                  spike::SetupPaths& paths) {
    const spike::ToolchainPin pin = fake_pin(kArchiveBytes);
    paths = spike::setup_paths(fresh_folder(folder_name), pin);
    return spike::install_toolchain(pin, paths, fake_tools(fakes));
}

void test_install_success() {
    Fakes fakes;
    spike::SetupPaths paths;
    const spike::InstallResult result = install_with(fakes, "ok", paths);
    SPIKE_EXPECT(result.ok && result.gcc_version == "gcc (fake) 16.2.0");
    SPIKE_EXPECT((fakes.calls == std::vector<std::string>{"download", "extract:fake.7z.exe", "probe"}));
    const auto recorded = spike::recorded_toolchain_gcc(paths.record_file);
    SPIKE_EXPECT(recorded && *recorded == result.gcc_path);
    SPIKE_EXPECT(!fs::exists(paths.toolchain_dir / "fake.7z.exe.part"));
}

void test_wrong_hash_is_e905_and_deleted() {
    Fakes fakes;
    fakes.served = "gcc FAKE";
    spike::SetupPaths paths;
    const spike::InstallResult result = install_with(fakes, "bad_hash", paths);
    SPIKE_EXPECT(!result.ok && result.error.code == "E905");
    SPIKE_EXPECT((fakes.calls == std::vector<std::string>{"download"}));
    SPIKE_EXPECT(!fs::exists(paths.toolchain_dir / "fake.7z.exe.part"));
    SPIKE_EXPECT(!fs::exists(paths.toolchain_dir / "fake.7z.exe"));
    SPIKE_EXPECT(!fs::exists(paths.record_file));
    Fakes longer;
    longer.served = kArchiveBytes + "!";
    SPIKE_EXPECT(install_with(longer, "bad_size", paths).error.code == "E905");
}

void test_network_failure_is_e906() {
    Fakes fakes;
    fakes.download_error = "o endereço do servidor não foi encontrado";
    spike::SetupPaths paths;
    const spike::InstallResult result = install_with(fakes, "network", paths);
    SPIKE_EXPECT(!result.ok && result.error.code == "E906");
    SPIKE_EXPECT(result.error.message.find("verifique a conexão, proxy ou antivírus") !=
                 std::string::npos);
}

void test_extract_and_probe_failures_are_e907() {
    spike::SetupPaths paths;
    Fakes broken;
    broken.extract_error = "o extrator terminou com código 2";
    SPIKE_EXPECT(install_with(broken, "extract", paths).error.code == "E907");
    Fakes empty;
    empty.extract_creates_gcc = false;
    SPIKE_EXPECT(install_with(empty, "no_gcc", paths).error.code == "E907");
    Fakes silent;
    silent.gcc_runs = false;
    const spike::InstallResult result = install_with(silent, "probe", paths);
    SPIKE_EXPECT(result.error.code == "E907" && !fs::exists(paths.record_file));
}

void test_remove_stays_inside() {
    const spike::SetupPaths paths = spike::setup_paths(fresh_folder("remove"), fake_pin(kArchiveBytes));
    write_file(paths.toolchain_dir / "kit" / "bin" / "gcc.exe", "x");
    write_file(paths.base.parent_path() / "outro.txt", "nao apagar");
    std::string error;
    SPIKE_EXPECT(!spike::remove_toolchain(paths, paths.base.parent_path(), error));
    SPIKE_EXPECT(error.find("fica fora de") != std::string::npos);
    SPIKE_EXPECT(fs::exists(paths.base.parent_path() / "outro.txt"));
    SPIKE_EXPECT(!spike::remove_toolchain(paths, paths.base, error));
    SPIKE_EXPECT(spike::remove_toolchain(paths, paths.base / "toolchain", error));
    SPIKE_EXPECT(!fs::exists(paths.base / "toolchain") && fs::exists(paths.base));
}

void test_setup_flags() {
    const char* check[] = {"spike", "setup", "--check"};
    const spike::CliArgs a = spike::parse_args(3, const_cast<char**>(check));
    SPIKE_EXPECT(a.command == spike::Command::SETUP && a.setup_check && !a.parse_error);
    const char* both[] = {"spike", "setup", "--force", "--yes"};
    const spike::CliArgs b = spike::parse_args(4, const_cast<char**>(both));
    SPIKE_EXPECT(b.setup_force && b.setup_yes && !b.parse_error);
    const char* clash[] = {"spike", "setup", "--remove", "--force"};
    SPIKE_EXPECT(spike::parse_args(4, const_cast<char**>(clash)).parse_error);
    const char* unknown[] = {"spike", "setup", "--depressa"};
    SPIKE_EXPECT(spike::parse_args(3, const_cast<char**>(unknown)).parse_error);
}

} // namespace

namespace spike_tests {

void run_setup_tests() {
    test_linux_families();
    test_paths_and_pin();
    test_is_inside();
    test_recorded_toolchain();
    test_install_success();
    test_wrong_hash_is_e905_and_deleted();
    test_network_failure_is_e906();
    test_extract_and_probe_failures_are_e907();
    test_remove_stays_inside();
    test_setup_flags();
}

} // namespace spike_tests
