# Contributing to Spike

Thanks for helping. Spike is a Portugol compiler for students in their first programming course, so every change is judged by one question: does it make things clearer for a beginner?

## Building

You need CMake 3.16 or newer, a C++17 compiler and a C compiler (GCC or Clang) for the tests that compile generated programs.

```
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

- **Linux:** `sudo apt install build-essential cmake git`.
- **macOS:** `xcode-select --install`, plus CMake from cmake.org or Homebrew.
- **Windows:** MSYS2 with the UCRT64 toolchain (`pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja`). Add `-G Ninja` to the first command, and put `C:\msys64\ucrt64\bin` before any other MinGW folder in `PATH`; otherwise the tests fail with `0xc0000139`.

The compiler ends up in `build/spike` (`build/spike.exe` on Windows). `-DSPIKE_SANITIZE=ON` builds with AddressSanitizer and UndefinedBehaviorSanitizer.

## Tests

- `ctest --test-dir build --output-on-failure` runs everything. It must be green, with zero compiler warnings, before a change goes in.
- `build/spike_tests` runs the unit tests and prints `all tests passed`. Never change that sentence: CTest looks for it.
- `ctest --test-dir build -L e2e` runs the end-to-end cases in `tests/e2e/`, real programs with input and expected output. `tests/e2e/README.md` explains how to add one.
- `ctest --test-dir build -R docs_examples` checks the code examples in the documentation (see below).

Every new behaviour gets a test. Every bug fix gets a test that failed before the fix. Every new language feature gets an e2e case with the expected output worked out by hand.

How the compiler is organized is in [docs/internals.md](docs/internals.md).

## Style

- C++17, no compiler extensions, `namespace spike`, `.hpp` and `.cpp`. `.clang-format` holds the formatting.
- Zero warnings with `-Wall -Wextra -Wpedantic`.
- No comments in new code. Pick names that make them unnecessary. Write a comment only when the reason is not obvious and does not fit in a name, and keep comments in ASCII.
- No file longer than 300 lines, code or test. When a file grows past that, split it by topic inside its module folder (`src/<module>/`, `tests/<module>/`). CTest's `check_file_sizes` enforces this.
- Code, file names, tests, commits and technical docs are in English. Everything a Spike user reads (errors, hints, warnings, CLI help, user docs) is in Brazilian Portuguese, with accents.
- Never start a program through a shell. Use `run_process` with an argument list; `system()` and `popen()` are not allowed in `src/`.
- No new external dependency without discussing it first in an issue.
- Every new error code gets an entry in `docs/errors.md` (message, when it happens, how to fix, an example). The build reads that file for `spike --explain`.

## Commits and pull requests

- One topic per commit. A change that mixes a refactor with a feature is two commits.
- Messages follow [Conventional Commits](https://www.conventionalcommits.org/), in English, with a title of at most 72 characters: `feat: add escolha statement`, `fix: report E401 once for nested blocks`, `docs: explain --c in the tutorial`.
- Refactors do not change behaviour: the whole suite passes before and after, and the number of tests does not go down.
- For anything bigger than a small fix, open an issue first and describe what you want to change. Language changes start with a proposal in `docs/language.md`.
- A new language feature follows the checklist in `CLAUDE.md` (spec, lexer, AST, parser, analyzer, generator, runtime, examples and e2e, error docs, changelog). `docs/internals.md` walks through a real example.

## Reporting a bug

Open an issue at [github.com/joaopssx/spike-compiler/issues](https://github.com/joaopssx/spike-compiler/issues) with:

1. The output of `spike fetch`. It shows the Spike version, the system and the C compiler Spike found.
2. The smallest `.por` file that shows the problem.
3. The exact command you ran, what you expected and what happened (paste the full message).

An `E903` error ("o compilador C recusou o código gerado") is always a Spike bug. Run the same command with `--c` and attach the generated `.c` file too.

## Code examples in the documentation

The examples in `README.md` and in every `.md` under `docs/` are tested. `spike_doctest` (in `tools/`, CTest `docs_examples`) reads the fenced code blocks and checks the ones marked with these languages:

| Fence | What is checked |
| --- | --- |
| ` ```portugol ` | `spike check` must succeed |
| ` ```portugol-run ` | the program is compiled and run; it must exit with 0 |
| ` ```input ` | right after a `portugol-run` block: fed to the program as its input |
| ` ```output ` | right after the `portugol-run` block (or its `input`): must equal what the program printed, ignoring trailing blank lines |
| ` ```portugol-error E700 ` | `spike check` must fail and report that code |

Blocks with any other language, or none, are ignored, so terminal sessions and C snippets stay as plain blocks. Fences must start at the beginning of the line. Programs run with `NO_COLOR=1` and `SPIKE_SEED=2026`, so `aleatorio` gives the same numbers every time. Without a C compiler, `portugol-run` blocks are only checked with `spike check`.

When you add an example to the docs, mark it so it is tested. Commands and outputs shown as plain text must be real: run them and paste what came out.
