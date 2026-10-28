#include "spike/setup.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace spike {

namespace {

std::string unquote(std::string value) {
    if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') &&
        value.back() == value.front()) {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::vector<std::string> words(const std::string& text) {
    std::istringstream in(text);
    std::vector<std::string> out;
    for (std::string word; in >> word;) out.push_back(word);
    return out;
}

LinuxFamily family_of(const std::string& id) {
    if (id == "debian" || id == "ubuntu" || id == "linuxmint" || id == "pop") {
        return LinuxFamily::Debian;
    }
    if (id == "fedora" || id == "rhel" || id == "centos" || id == "rocky" || id == "almalinux") {
        return LinuxFamily::Fedora;
    }
    if (id == "arch" || id == "manjaro" || id == "endeavouros") return LinuxFamily::Arch;
    if (id == "suse" || id == "opensuse" || id.rfind("opensuse-", 0) == 0 || id == "sles") {
        return LinuxFamily::OpenSuse;
    }
    if (id == "alpine") return LinuxFamily::Alpine;
    return LinuxFamily::Unknown;
}

} // namespace

LinuxFamily detect_linux_family(const std::string& os_release_text) {
    std::istringstream in(os_release_text);
    std::string id;
    std::string id_like;
    for (std::string line; std::getline(in, line);) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("ID=", 0) == 0) id = unquote(line.substr(3));
        if (line.rfind("ID_LIKE=", 0) == 0) id_like = unquote(line.substr(8));
    }
    std::vector<std::string> candidates = {id};
    for (const std::string& like : words(id_like)) candidates.push_back(like);
    for (const std::string& candidate : candidates) {
        const LinuxFamily family = family_of(candidate);
        if (family != LinuxFamily::Unknown) return family;
    }
    return LinuxFamily::Unknown;
}

std::string linux_install_command(LinuxFamily family) {
    switch (family) {
        case LinuxFamily::Debian: return "sudo apt install build-essential";
        case LinuxFamily::Fedora: return "sudo dnf install gcc";
        case LinuxFamily::Arch: return "sudo pacman -S gcc";
        case LinuxFamily::OpenSuse: return "sudo zypper install gcc";
        case LinuxFamily::Alpine: return "sudo apk add build-base";
        case LinuxFamily::Unknown: break;
    }
    return "Debian/Ubuntu:  sudo apt install build-essential\n"
           "Fedora:         sudo dnf install gcc\n"
           "Arch:           sudo pacman -S gcc\n"
           "openSUSE:       sudo zypper install gcc\n"
           "Alpine:         sudo apk add build-base";
}

} // namespace spike
