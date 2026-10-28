#ifndef _WIN32

#include "spike/process.hpp"

#include <cerrno>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace spike {

namespace {

struct Pipe {
    int read_end = -1;
    int write_end = -1;
};

void close_fd(int& fd) {
    if (fd >= 0) close(fd);
    fd = -1;
}

bool open_pipe(Pipe& p) {
    int fds[2];
    if (pipe(fds) != 0) return false;
    p.read_end = fds[0];
    p.write_end = fds[1];
    return true;
}

void drain(int& out_fd, std::string& out, int& err_fd, std::string& err) {
    char buffer[65536];
    while (out_fd >= 0 || err_fd >= 0) {
        pollfd fds[2];
        nfds_t count = 0;
        if (out_fd >= 0) fds[count++] = pollfd{out_fd, POLLIN, 0};
        if (err_fd >= 0) fds[count++] = pollfd{err_fd, POLLIN, 0};
        if (poll(fds, count, -1) < 0) {
            if (errno == EINTR) continue;
            break;
        }
        for (nfds_t i = 0; i < count; ++i) {
            if (fds[i].revents == 0) continue;
            const bool is_out = fds[i].fd == out_fd;
            const ssize_t n = read(fds[i].fd, buffer, sizeof buffer);
            if (n > 0) {
                (is_out ? out : err).append(buffer, static_cast<std::size_t>(n));
            } else if (n == 0 || errno != EINTR) {
                close_fd(is_out ? out_fd : err_fd);
            }
        }
    }
    close_fd(out_fd);
    close_fd(err_fd);
}

int wait_for(pid_t pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) return -1;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return -1;
}

} // namespace

ProcessResult run_process(const ProcessOptions& options) {
    ProcessResult result;
    if (options.args.empty()) {
        result.launch_error = "nenhum programa informado";
        return result;
    }
    if (options.working_dir) {
        result.launch_error = "working_dir só é usado no Windows";
        return result;
    }
    const std::optional<std::string> program = find_executable(options.args[0]);
    if (!program) {
        result.launch_error = "programa não encontrado: '" + options.args[0] + "'";
        return result;
    }

    int stdin_fd = -1;
    if (options.stdin_file) {
        stdin_fd = open(options.stdin_file->c_str(), O_RDONLY);
        if (stdin_fd < 0) {
            result.launch_error = "não foi possível abrir '" + *options.stdin_file + "': " +
                                  std::strerror(errno);
            return result;
        }
    }
    Pipe out_pipe;
    Pipe err_pipe;
    if ((options.capture_stdout && !open_pipe(out_pipe)) ||
        (options.capture_stderr && !open_pipe(err_pipe))) {
        result.launch_error = std::string("não foi possível criar um pipe: ") + std::strerror(errno);
        close_fd(stdin_fd);
        for (Pipe* p : {&out_pipe, &err_pipe}) {
            close_fd(p->read_end);
            close_fd(p->write_end);
        }
        return result;
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    if (stdin_fd >= 0) {
        posix_spawn_file_actions_adddup2(&actions, stdin_fd, STDIN_FILENO);
        posix_spawn_file_actions_addclose(&actions, stdin_fd);
    }
    for (Pipe* p : {&out_pipe, &err_pipe}) {
        if (p->write_end < 0) continue;
        posix_spawn_file_actions_adddup2(&actions, p->write_end,
                                         p == &out_pipe ? STDOUT_FILENO : STDERR_FILENO);
        posix_spawn_file_actions_addclose(&actions, p->write_end);
        posix_spawn_file_actions_addclose(&actions, p->read_end);
    }

    std::vector<char*> argv;
    argv.reserve(options.args.size() + 1);
    argv.push_back(const_cast<char*>(program->c_str()));
    for (std::size_t i = 1; i < options.args.size(); ++i) {
        argv.push_back(const_cast<char*>(options.args[i].c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = 0;
    const int spawn_error = posix_spawn(&pid, program->c_str(), &actions, nullptr,
                                        argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    close_fd(stdin_fd);
    close_fd(out_pipe.write_end);
    close_fd(err_pipe.write_end);
    if (spawn_error != 0) {
        close_fd(out_pipe.read_end);
        close_fd(err_pipe.read_end);
        result.launch_error = "não foi possível executar '" + options.args[0] + "': " +
                              std::strerror(spawn_error);
        return result;
    }

    result.launched = true;
    drain(out_pipe.read_end, result.out, err_pipe.read_end, result.err);
    result.exit_code = wait_for(pid);
    return result;
}

} // namespace spike

#endif
