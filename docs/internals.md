# Spike internals

Notes for people working on the compiler itself. The user-facing language is described in `language.md`; the C runtime in `runtime.md`.

## Process execution

Spike has to start other programs: the C compiler, and with `--run`, the program it just built. All of that goes through one function, `run_process` in `include/spike/process.hpp`, and it never involves a shell. There is no `system()`, no `popen()`, no `sh -c` and no `cmd /c` anywhere in `src/`.

### Why no shell

A shell reads a command line as a small program. Quotes, `;`, `&`, `|`, `$(...)`, backticks and `%VAR%` all mean something. If Spike built a command line by gluing strings together, a file named `a; rm -rf ~.por` or `x & del *.por` would run commands the student never asked for. Escaping rules differ between `sh`, `cmd.exe` and PowerShell, so "escape it carefully" is not a fix.

Instead, `run_process` takes the arguments as a list (`ProcessOptions::args`, with the program in `args[0]`) and hands that list to the operating system directly. Each argument reaches the child exactly as written. The test `test_process_arguments_arrive_intact` proves it with a helper program, `spike_echo_args`, that prints every argument it receives: spaces, quotes, `;`, `$(whoami)`, `%PATH%`, `&&`, `|`, `>`, trailing backslashes and accented letters all come back unchanged.

### Finding the program

`find_executable` resolves `args[0]` before anything runs. A name with a directory part is used as is; a bare name is looked up in `PATH` by Spike itself, directory by directory. On Windows `.exe` is tried first. Empty `PATH` entries are skipped, so the current directory is never searched by accident. The child is then started with the full path, which also stops Windows from looking in the current directory first.

### POSIX

`posix_spawn` with file actions: the optional stdin file is opened by Spike and `dup2`'d onto the child's stdin, and capture pipes are `dup2`'d onto stdout and stderr. Spike reads both pipes with `poll`, so a child that fills one pipe while Spike waits on the other cannot deadlock. `waitpid` gives the exit code; a child killed by a signal reports `128 + signal`, like shells do.

### Windows

`CreateProcessW` with the resolved path as the application name and a command line built with the quoting rules that `CommandLineToArgvW` and the C runtime use to split it again: arguments with spaces, tabs or quotes are wrapped in quotes, quotes are escaped with a backslash, and backslashes are doubled only when they come before a quote. All strings are UTF-8 inside Spike and converted to UTF-16 at the API boundary, so paths like `C:\Users\João` work. Capture uses inheritable pipes whose read ends are not inherited; stderr is drained on a second thread while stdout is read on the calling thread.

### Errors

If the program cannot be found or started, `launched` is false and `launch_error` says why, in Portuguese. A started program that fails just reports its `exit_code`.

## C compiler driver

`include/spike/cc_driver.hpp`.

- `find_c_compiler(bundled_dir)` looks in this order: the `SPIKE_CC` environment variable (if set and invalid, the search stops and the result is E902, because the user asked for that compiler); `bundled_dir/bin/{gcc,clang,cc}`, for the compiler that will ship with Spike; then `cc`, `gcc`, `clang` in `PATH` (`gcc`, `clang` on Windows). `kind` comes from the file name.
- `compile_c` runs `[compiler, -O2 or -O0, -std=c99, -o, output, c_file, -lm]` with both output streams captured. It never adds `-Werror`: warnings in generated code are Spike's problem, not the student's.
- Errors come back as data (`DriverError{code, message, hint, details}`); the CLI prints them. E902: no compiler. E903: the compiler rejected the generated C, which is always a Spike bug; `details` holds the first 20 lines of compiler output. E904: a file could not be written.

## Temporary files

`TempDir` (`include/spike/temp_dir.hpp`) creates a uniquely named folder under the system temp directory (`spike-<pid>-<random>`) and deletes it with everything inside when it goes out of scope, even on an error path. The generated `.c` lives there; with `--c` it is copied next to the executable before the folder is removed.
