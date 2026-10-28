#ifdef _WIN32

#include "spike/process.hpp"
#include "process/utf8_path.hpp"

#include <functional>
#include <string>
#include <thread>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace spike {

using process_detail::from_wide;
using process_detail::to_wide;

namespace {

std::wstring quote_argument(const std::wstring& arg) {
    if (!arg.empty() && arg.find_first_of(L" \t\n\v\"") == std::wstring::npos) return arg;
    std::wstring out = L"\"";
    for (auto it = arg.begin();; ++it) {
        std::size_t backslashes = 0;
        while (it != arg.end() && *it == L'\\') {
            ++it;
            ++backslashes;
        }
        if (it == arg.end()) {
            out.append(backslashes * 2, L'\\');
            break;
        }
        if (*it == L'"') {
            out.append(backslashes * 2 + 1, L'\\');
        } else {
            out.append(backslashes, L'\\');
        }
        out.push_back(*it);
    }
    out.push_back(L'"');
    return out;
}

std::wstring build_command_line(const std::wstring& program, const std::vector<std::string>& args) {
    std::wstring line = L"\"" + program + L"\"";
    for (std::size_t i = 1; i < args.size(); ++i) {
        line += L' ';
        line += quote_argument(to_wide(args[i]));
    }
    return line;
}

std::string error_text(DWORD code) {
    wchar_t* buffer = nullptr;
    const DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, 0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);
    std::string text = size > 0 ? from_wide(std::wstring(buffer, size)) : std::string();
    if (buffer) LocalFree(buffer);
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) {
        text.pop_back();
    }
    return text.empty() ? "erro " + std::to_string(code) : text;
}

void close_handle(HANDLE& handle) {
    if (handle && handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
    handle = nullptr;
}

HANDLE inheritable_copy(DWORD which) {
    const HANDLE original = GetStdHandle(which);
    if (!original || original == INVALID_HANDLE_VALUE) return nullptr;
    HANDLE copy = nullptr;
    if (!DuplicateHandle(GetCurrentProcess(), original, GetCurrentProcess(), &copy, 0, TRUE,
                         DUPLICATE_SAME_ACCESS)) {
        return nullptr;
    }
    return copy;
}

bool open_pipe(HANDLE& read_end, HANDLE& write_end) {
    SECURITY_ATTRIBUTES attributes{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    if (!CreatePipe(&read_end, &write_end, &attributes, 0)) return false;
    SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);
    return true;
}

void read_all(HANDLE handle, std::string& out) {
    char buffer[65536];
    DWORD count = 0;
    while (ReadFile(handle, buffer, sizeof buffer, &count, nullptr) && count > 0) {
        out.append(buffer, count);
    }
}

} // namespace

ProcessResult run_process(const ProcessOptions& options) {
    ProcessResult result;
    if (options.args.empty()) {
        result.launch_error = "nenhum programa informado";
        return result;
    }
    const std::optional<std::string> program = find_executable(options.args[0]);
    if (!program) {
        result.launch_error = "programa não encontrado: '" + options.args[0] + "'";
        return result;
    }

    HANDLE child_in = nullptr;
    HANDLE child_out = nullptr;
    HANDLE child_err = nullptr;
    HANDLE out_read = nullptr;
    HANDLE err_read = nullptr;
    auto close_all = [&]() {
        for (HANDLE* h : {&child_in, &child_out, &child_err, &out_read, &err_read}) close_handle(*h);
    };

    if (options.stdin_file) {
        SECURITY_ATTRIBUTES attributes{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        child_in = CreateFileW(to_wide(*options.stdin_file).c_str(), GENERIC_READ,
                               FILE_SHARE_READ, &attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                               nullptr);
        if (child_in == INVALID_HANDLE_VALUE) {
            child_in = nullptr;
            result.launch_error = "não foi possível abrir '" + *options.stdin_file + "': " +
                                  error_text(GetLastError());
            return result;
        }
    } else {
        child_in = inheritable_copy(STD_INPUT_HANDLE);
    }
    const bool pipes_ok =
        (options.capture_stdout ? open_pipe(out_read, child_out)
                                : (child_out = inheritable_copy(STD_OUTPUT_HANDLE), true)) &&
        (options.capture_stderr ? open_pipe(err_read, child_err)
                                : (child_err = inheritable_copy(STD_ERROR_HANDLE), true));
    if (!pipes_ok) {
        result.launch_error = "não foi possível criar um pipe: " + error_text(GetLastError());
        close_all();
        return result;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof startup;
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = child_in;
    startup.hStdOutput = child_out;
    startup.hStdError = child_err;
    PROCESS_INFORMATION info{};
    const std::wstring application = to_wide(*program);
    std::wstring command_line = build_command_line(application, options.args);
    const std::wstring working_dir = options.working_dir ? to_wide(*options.working_dir) : L"";
    const BOOL created = CreateProcessW(application.c_str(), command_line.data(), nullptr, nullptr,
                                        TRUE, 0, nullptr,
                                        options.working_dir ? working_dir.c_str() : nullptr,
                                        &startup, &info);
    const DWORD create_error = GetLastError();
    close_handle(child_in);
    close_handle(child_out);
    close_handle(child_err);
    if (!created) {
        result.launch_error = "não foi possível executar '" + options.args[0] + "': " +
                              error_text(create_error);
        close_all();
        return result;
    }

    result.launched = true;
    CloseHandle(info.hThread);
    std::thread err_reader;
    if (err_read) err_reader = std::thread(read_all, err_read, std::ref(result.err));
    if (out_read) read_all(out_read, result.out);
    if (err_reader.joinable()) err_reader.join();
    close_all();

    WaitForSingleObject(info.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(info.hProcess, &exit_code);
    CloseHandle(info.hProcess);
    result.exit_code = static_cast<int>(exit_code);
    return result;
}

} // namespace spike

#endif
