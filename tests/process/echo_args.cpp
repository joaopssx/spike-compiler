#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <fcntl.h>
#include <io.h>
#include <shellapi.h>
#endif

namespace {

std::vector<std::string> arguments(int argc, char** argv) {
    std::vector<std::string> out;
#ifdef _WIN32
    (void)argc;
    (void)argv;
    int count = 0;
    wchar_t** wide = CommandLineToArgvW(GetCommandLineW(), &count);
    for (int i = 0; i < count; ++i) {
        const int size = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
        std::string arg(static_cast<std::size_t>(size > 0 ? size - 1 : 0), '\0');
        if (size > 1) {
            WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, arg.data(), size, nullptr, nullptr);
        }
        out.push_back(arg);
    }
    LocalFree(wide);
#else
    for (int i = 0; i < argc; ++i) out.emplace_back(argv[i]);
#endif
    return out;
}

void use_binary_streams() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
}

} // namespace

int main(int argc, char** argv) {
    use_binary_streams();
    const std::vector<std::string> args = arguments(argc, argv);
    if (args.size() >= 3 && args[1] == "--exit") return std::atoi(args[2].c_str());
    if (args.size() >= 2 && args[1] == "--cat") {
        int c = 0;
        while ((c = std::getchar()) != EOF) std::putchar(c);
        return 0;
    }
    if (args.size() >= 2 && args[1] == "--big") {
        const std::string out_chunk(1024, 'o');
        const std::string err_chunk(1024, 'e');
        for (int i = 0; i < 1024; ++i) {
            std::fwrite(out_chunk.data(), 1, out_chunk.size(), stdout);
            std::fwrite(err_chunk.data(), 1, err_chunk.size(), stderr);
        }
        return 0;
    }
    for (std::size_t i = 1; i < args.size(); ++i) std::printf("%s\n", args[i].c_str());
    return 0;
}
