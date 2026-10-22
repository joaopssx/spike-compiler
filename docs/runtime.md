# The C runtime

Every program Spike generates is a single C file. Its first part is the runtime, a header-only C99 library with the helpers the generated code calls. It lives in four files: `runtime/spike_runtime_core.h` (types, errors, memory, texts, arithmetic, `para` helpers), `runtime/spike_runtime_calls.h` (call counting for procedures and functions), `runtime/spike_runtime_math.h` (the math built-ins) and `runtime/spike_runtime_io.h` (writing, reading, parsing). `runtime/spike_runtime.h` includes them in that order, for the C tests; the later files rely on `core` being included first. Everything in it is `static inline`, so the C compiler drops what a program does not use and the result is still one standalone executable.

## Rules

- C99 and the standard library only. On Windows it also includes `windows.h`, to switch the console to UTF-8 so accented letters print and read correctly and to measure the stack. On Linux and macOS it includes `sys/resource.h` for the stack size. The driver links Windows executables with a 16 MB stack (`-Wl,--stack,16777216`).
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
| `spike_pow(base, exponent, line)` | `^`. Calls `pow` from `<math.h>` (the driver links with `-lm`); a NaN or infinite result is R005. |
| `spike_index(index, low, high, line, name)` | Turns a Portugol index into a C offset (`index - low`); an index outside `low..high` is R002, with the vector's name and limits in the message. The generated code evaluates the index once, as an argument. |
| `spike_index_dim(index, low, high, line, name, dimension)` | The same check for one index of a matrix. The message names the dimension (`índice 5 fora da dimensão 2 de 'm'`). The generated code combines the offsets in row order: `(spike_index_dim(i, ...) * 4) + spike_index_dim(j, ...)`. |
| `spike_fill_str(items, count)` | Sets every element of a `caractere` vector to `""`. The generated `main` calls it for each such vector before anything else, since C would leave them as null pointers. |
| `spike_check_step(step, line)` | R006 if the step of a `para` is 0. Generated only when the step is not a literal. |
| `spike_for_advance(&counter, step)` | Adds the step to a `para` counter and returns 1, or returns 0 without changing it when the sum would overflow. The generated loop `break`s on 0, so a `para` near the `inteiro` limits never loops forever. |
| `spike_enter(line)` | First line of every generated procedure and function. Counts the call: more than 10000 calls in progress is R008. On the first call it also measures the stack (`getrlimit` on Linux and macOS, `VirtualQuery` on Windows) and allows recursion to use 3/4 of it; going past that is R008 too, so deep recursion stops with a message instead of crashing. `line` is the line of the procedure or function header. |
| `spike_leave()` | Undoes one `spike_enter`. Generated before every exit: each `retorne` and the end of a procedure. |
| `spike_missing_return(line, name)` | R009: the function `name` reached `fimfuncao` without `retorne`. Generated at the end of every function, followed by a `return` of the type's default value so the C compiler sees every path return. |
| `spike_abs_int(x)` | `abs` of an `inteiro`. `abs` of the smallest `inteiro` wraps to itself instead of being undefined in C. `abs` of a `real` calls `fabs` directly. |
| `spike_sqrt(x, line)` | `raiz`; a negative `x` is R005. |
| `spike_log(x, line)` | `log`, base 10 (`log10`); zero, negative or NaN is R005. |
| `spike_real_to_int(x, line)` | Converts a real to `inteiro`. NaN, or a value outside the 64-bit range, is R004. |
| `spike_round`, `spike_trunc`, `spike_floor`, `spike_ceil` | `arredonda` (`round`, halves away from zero), `trunca`, `piso` and `teto`, each followed by `spike_real_to_int`. |
| `spike_random(low, high, line)` | `aleatorio`: a uniform integer from `low` to `high`, both included, without modulo bias. `low > high` is R010. The generator is splitmix64, so a seed gives the same numbers on every system. |
| `spike_seed_random(seed)` | Sets the seed. The first `spike_random` seeds itself from `SPIKE_SEED` when that environment variable holds a number, or from `time(NULL)` otherwise. |
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
| R002 | A vector index is outside the declared limits. |
| R003 | `leia` got text that does not fit the variable's type. |
| R004 | A real number does not fit in an `inteiro` (`arredonda`, `trunca`, `piso`, `teto`). |
| R005 | `^` or `pot` produced a result that is not a number or is infinite; `raiz` of a negative; `log` of zero or less. |
| R006 | The step of a `para` is 0 when the loop starts. |
| R007 | The input ended before `leia` could read a line. |
| R008 | Out of memory, or recursion deeper than 10000 calls or than the stack allows. |
| R009 | A function ended without `retorne`. |
| R010 | `aleatorio` got its limits in the wrong order. |

A program that stops on a runtime error always exits with code 101.

## How it gets into the compiler

`cmake/embed_runtime.cmake` runs at build time (`cmake -P`). It reads `spike_runtime_core.h`, `spike_runtime_calls.h`, `spike_runtime_math.h` and `spike_runtime_io.h`, joins them in that order, turns `\r\n` into `\n`, and writes `build/generated/runtime_text.cpp` with the file as a byte array. A byte array avoids the length limit some compilers put on string literals. `spike_core` compiles that file, and `include/spike/runtime_text.hpp` exposes it as `runtime_source()` and `runtime_source_size()`. The generated file is rebuilt whenever the header or the script changes, and it is never committed.

## Tests

- `runtime_c_unit` compiles `tests/runtime/runtime_test.c` as C99 and checks formatting, texts, arithmetic and parsing.
- `runtime_io_*` run `tests/runtime/runtime_io_program.c` with real input through `tests/runtime/expect_run.cmake`, which checks the exit code, stdout and stderr. They cover reading every type, invalid input (R003), end of input (R007), division by zero (R001), the call limit (R008), a function without `retorne` (R009), the math errors (R004, R005, R010) and `SPIKE_SEED`.
- The C tests are skipped when CMake finds no C compiler.
