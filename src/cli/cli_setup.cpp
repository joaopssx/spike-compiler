#include "cli/cli_setup.hpp"

#include <cstring>
#include <string>

namespace spike {

bool parse_setup_args(int argc, char* argv[], CliArgs& out) {
    out.command = Command::SETUP;
    for (int i = 2; i < argc; ++i) {
        const std::string flag = argv[i];
        if (flag == "--check") {
            out.setup_check = true;
        } else if (flag == "--force") {
            out.setup_force = true;
        } else if (flag == "--remove") {
            out.setup_remove = true;
        } else if (flag == "--yes" || flag == "-y") {
            out.setup_yes = true;
        } else {
            out.parse_error = true;
            out.parse_error_msg = "flag desconhecida para 'setup': '" + flag +
                                  "'. use --check, --force, --remove ou --yes.";
            return false;
        }
    }
    const int modes = int{out.setup_check} + int{out.setup_force} + int{out.setup_remove};
    if (modes > 1) {
        out.parse_error = true;
        out.parse_error_msg = "use só uma destas flags de 'setup': --check, --force ou --remove.";
        return false;
    }
    return true;
}

} // namespace spike
