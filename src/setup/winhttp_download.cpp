#include "spike/setup.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <winhttp.h>
#endif

namespace spike {

namespace fs = std::filesystem;

#ifdef _WIN32

namespace {

class Handle {
public:
    explicit Handle(HINTERNET handle) : handle_(handle) {}
    ~Handle() {
        if (handle_) WinHttpCloseHandle(handle_);
    }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    HINTERNET get() const { return handle_; }
    explicit operator bool() const { return handle_ != nullptr; }

private:
    HINTERNET handle_;
};

std::wstring widen(const std::string& text) {
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (size <= 1) return std::wstring();
    std::wstring out(static_cast<std::size_t>(size - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), size);
    return out;
}

std::string last_error_text() {
    const DWORD code = GetLastError();
    switch (code) {
        case ERROR_WINHTTP_NAME_NOT_RESOLVED: return "o endereço do servidor não foi encontrado";
        case ERROR_WINHTTP_CANNOT_CONNECT: return "não foi possível conectar ao servidor";
        case ERROR_WINHTTP_TIMEOUT: return "a conexão demorou demais";
        case ERROR_WINHTTP_SECURE_FAILURE: return "o certificado HTTPS do servidor não é válido";
        case ERROR_WINHTTP_CONNECTION_ERROR: return "a conexão caiu no meio do download";
        default: return "erro do WinHTTP " + std::to_string(code);
    }
}

std::uint64_t content_length(HINTERNET request) {
    wchar_t buffer[32] = {};
    DWORD size = sizeof buffer;
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX,
                             buffer, &size, WINHTTP_NO_HEADER_INDEX)) {
        return 0;
    }
    return std::wcstoull(buffer, nullptr, 10);
}

} // namespace

std::string winhttp_download(const std::string& url, const fs::path& destination,
                             const DownloadProgress& progress) {
    std::wstring wide_url = widen(url);
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof parts;
    wchar_t host[256] = {};
    wchar_t path[2048] = {};
    parts.lpszHostName = host;
    parts.dwHostNameLength = 256;
    parts.lpszUrlPath = path;
    parts.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(wide_url.c_str(), 0, 0, &parts)) return "endereço inválido";
    if (parts.nScheme != INTERNET_SCHEME_HTTPS) return "o Spike só baixa por HTTPS";

    Handle session(WinHttpOpen(L"Spike-setup", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                               WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session) return last_error_text();
    DWORD policy = WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP;
    WinHttpSetOption(session.get(), WINHTTP_OPTION_REDIRECT_POLICY, &policy, sizeof policy);
    Handle connection(WinHttpConnect(session.get(), host, parts.nPort, 0));
    if (!connection) return last_error_text();
    Handle request(WinHttpOpenRequest(connection.get(), L"GET", path, nullptr,
                                      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      WINHTTP_FLAG_SECURE));
    if (!request) return last_error_text();
    if (!WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request.get(), nullptr)) {
        return last_error_text();
    }
    DWORD status = 0;
    DWORD status_size = sizeof status;
    WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
                        WINHTTP_NO_HEADER_INDEX);
    if (status != 200) return "o servidor respondeu HTTP " + std::to_string(status);

    std::ofstream out(destination, std::ios::binary | std::ios::trunc);
    if (!out) return "não consegui criar '" + destination.u8string() + "'";
    const std::uint64_t total = content_length(request.get());
    std::uint64_t done = 0;
    std::vector<char> buffer(1 << 16);
    while (true) {
        DWORD read = 0;
        if (!WinHttpReadData(request.get(), buffer.data(), static_cast<DWORD>(buffer.size()), &read)) {
            return last_error_text();
        }
        if (read == 0) break;
        out.write(buffer.data(), read);
        if (!out) return "não consegui gravar '" + destination.u8string() + "'";
        done += read;
        if (progress) progress(done, total);
    }
    return "";
}

#else

std::string winhttp_download(const std::string& url, const fs::path& destination,
                             const DownloadProgress& progress) {
    (void)url;
    (void)destination;
    (void)progress;
    return "o download automático só existe no Windows";
}

#endif

} // namespace spike
