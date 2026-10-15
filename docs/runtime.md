# The C runtime

Every program Spike generates is a single C file. Its first part is the runtime, a header-only C99 library with the helpers the generated code calls. It lives in two files, `runtime/spike_runtime_core.h` (types, errors, memory, texts, arithmetic, `para` helpers) and `runtime/spike_runtime_io.h` (writing, reading, parsing). `runtime/spike_runtime.h` includes both, in that order, for the C tests; `io` relies on `core` being included first. Everything in it is `static inline`, so the C compiler drops what a program does not use and the result is still one standalone executable.

## Rules

- C99 and the standard library only. On Windows it also includes `windows.h`, to switch the console to UTF-8 so accented letters print and read correctly.
- Every name starts with `spike_` or `SPIKE_`. Student variables get a different prefix from the code generator, so the two never collide.
- It compiles without warnings with `-Wall -Wextra -Wpedantic` on GCC and Clang.
- It never calls `setlocale`, so the decimal separator is always a point.

## Types

| Name | C type | Portugol type |
| --- | --- | --- |
| `spike_int` | `long long` | `inteiro` (64 bits) |
| `spike_real` | `double` | `real` |
| `spike_bool` | `int` | `logico` |
| `spike_str` | `const char*` | `caractere` |

Texts are immutable. Literals are plain C strings. Texts built while the program runs (joins, lines read with `leia`) live in blocks owned by the runtime, which `spike_cleanup` frees at exit.

## Functions

| Function | What it does |
| --- | --- |
| `spike_init()` | Switches the Windows console to UTF-8 and registers `spike_cleanup` to run at exit. The generated `main` calls it first. |
| `spike_cleanup()` | Frees runtime-owned texts and restores the console code page. Safe to call twice. |
| `spike_flush()` | Flushes stdout. Called before reading input and before exiting. |
| `spike_fail(code, line, message)` | Prints `erro de execução[Rxxx]: message` and `  --> linha N` to stderr, cleans up and exits with 101. A line of 0 omits the second line. |
| `spike_str_copy(s)` | Runtime-owned copy of `s`. |
| `spike_str_concat(a, b)` | `a` followed by `b`, runtime-owned. |
| `spike_str_eq(a, b)` | 1 if the texts are equal. |
| `spike_str_cmp(a, b)` | -1, 0 or 1, comparing byte by byte. |
| `spike_add_int`, `spike_sub_int`, `spike_mul_int` | `+`, `-` and `*` on `inteiro`. They compute in unsigned arithmetic and convert back, so overflow wraps around instead of being undefined behavior in C. |
| `spike_div_int(a, b, line)` | Integer division truncated toward zero; R001 if `b` is 0. `LLONG_MIN div -1` wraps instead of crashing. |
| `spike_mod_int(a, b, line)` | Remainder with the sign of `a`, as in C; R001 if `b` is 0. |
| `spike_div_real(a, b, line)` | Real division; R001 if `b` is 0. |
| `spike_check_step(step, line)` | R006 if the step of a `para` is 0. Generated only when the step is not a literal. |
| `spike_for_advance(&counter, step)` | Adds the step to a `para` counter and returns 1, or returns 0 without changing it when the sum would overflow. The generated loop `break`s on 0, so a `para` near the `inteiro` limits never loops forever. |
| `spike_write_int(v)` | Prints an integer. |
| `spike_write_real(v)` | Prints a real with `%.10g`: `3` for 3.0, `3.5`, `0.3` for 0.1 + 0.2. Negative zero prints `0`. |
| `spike_write_str(v)` | Prints a text. |
| `spike_write_bool(v)` | Prints `verdadeiro` or `falso`. |
| `spike_write_newline()` | Prints a line break. |
| `spike_read_int(line, name)` | Reads a line and parses an integer (spaces around it are fine). |
| `spike_read_real(line, name)` | Reads a line and parses a real; accepts `3.5` and `3,5`. |
| `spike_read_str(line, name)` | Reads a whole line, without the line break (`\r\n` or `\n`). |
| `spike_read_bool(line, name)` | Reads `verdadeiro`, `falso`, `v`, `f`, `sim` or `nao`, in any case. |
| `spike_format_real`, `spike_parse_int`, `spike_parse_real`, `spike_parse_bool` | The pure helpers behind the functions above, exposed so they can be tested without a console. |

Every read function takes the `.por` line of the `leia` and the variable name as written, so error messages can point at both: R003 shows what was typed (cut at 40 bytes, never in the middle of a UTF-8 character), R007 names the variable. Reading flushes stdout first, so a prompt written with `escreva` shows up before the program waits. The first read skips a UTF-8 byte order mark if the input starts with one.

## Runtime errors

| Code | Meaning |
| --- | --- |
| R001 | Division by zero in `/`, `div` or `mod`. |
| R003 | `leia` got text that does not fit the variable's type. |
| R006 | The step of a `para` is 0 when the loop starts. |
| R007 | The input ended before `leia` could read a line. |
| R008 | Out of memory. |

R002, R004 and R005 are reserved by `language.md` for vectors, conversions and math functions, and arrive with those features. A program that stops on a runtime error always exits with code 101.

## How it gets into the compiler

`cmake/embed_runtime.cmake` runs at build time (`cmake -P`). It reads `spike_runtime_core.h` and `spike_runtime_io.h`, joins them in that order, turns `\r\n` into `\n`, and writes `build/generated/runtime_text.cpp` with the file as a byte array. A byte array avoids the length limit some compilers put on string literals. `spike_core` compiles that file, and `include/spike/runtime_text.hpp` exposes it as `runtime_source()` and `runtime_source_size()`. The generated file is rebuilt whenever the header or the script changes, and it is never committed.

## Tests

- `runtime_c_unit` compiles `tests/runtime/runtime_test.c` as C99 and checks formatting, texts, arithmetic and parsing.
- `runtime_io_*` run `tests/runtime/runtime_io_program.c` with real input through `tests/runtime/expect_run.cmake`, which checks the exit code, stdout and stderr. They cover reading every type, invalid input (R003), end of input (R007) and division by zero (R001).
- The C tests are skipped when CMake finds no C compiler.
