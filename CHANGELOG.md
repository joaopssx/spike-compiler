# Changelog

All notable changes to Spike are recorded here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `div` (integer division, `7 div 2` is `3`), `mod` (remainder, `7 mod 2` is `1`) and `^` (power, `2 ^ 10` is `1024`). `^` binds tighter than the unary minus and groups to the right, so `-2 ^ 2` is `-4` and `2 ^ 3 ^ 2` is `512`.
- Runtime error R005 when a power has no real result, such as `0 ^ -1`.
- Constants: a `const` section before `var`, with lines like `PI = 3.14159`. Spike computes their values while compiling, and changing a constant is an error (E740).
- `interrompa`, which leaves the innermost `enquanto`, `repita` or `para` right away. Using it outside a loop is an error (E750).
- `escolha` / `caso` / `outrocaso` / `fimescolha`, with lists (`caso 2, 3`) and ranges (`caso 4 ate 6`). Only the matching case runs, and the value can be an `inteiro` or a `caractere`.
- Vectors: `notas : vetor[1..5] de real`, used as `notas[i]` in expressions, assignments and `leia`. Limits can be negative or constants. An index outside the limits stops the program with runtime error R002, or is reported while compiling when the index is a fixed value (E774).
- Matrices: vectors with 2 or 3 dimensions, `m : vetor[1..3, 1..4] de inteiro`, used as `m[i, j]`. Each index is checked against its own dimension, and the runtime error says which one failed.
- Procedures: `procedimento nome(a: inteiro, b: real)` ... `fimprocedimento`, declared before the main `inicio` and called as a command, `nome(1, 2.5)`. Parameters are passed by value. A procedure can have its own `var` section, whose variables start fresh on every call and may reuse the name of a global. Procedures can call each other in any order and call themselves.
- Functions: `funcao fatorial(n: inteiro): inteiro` ... `fimfuncao`, with `retorne valor`. A call works anywhere a value does, as in `escreval(fatorial(5))` or `se primo(n) entao`. Inside a procedure, `retorne` alone leaves early.
- Math built-in functions: `abs`, `raiz`, `pot`, `arredonda`, `trunca`, `piso`, `teto`, `seno`, `cosseno`, `tangente`, `log`, `exp` and `aleatorio`. Runtime errors: `raiz` of a negative and `log` of zero or less are R005, a real too big for `arredonda` and friends is R004, and `aleatorio` with its limits reversed is the new R010. Setting `SPIKE_SEED` makes `aleatorio` repeat the same numbers.
- Text built-in functions: `tamanho`, `maiusculo`, `minusculo`, `copia`, `pos`, `concatena`, `paraTexto`, `paraInteiro`, `paraReal`, `ehNumero`, `ord` and `chr`. They count characters, not bytes, so `tamanho("ação")` is 4. Invalid text in `paraInteiro` or `paraReal` is R004, and a position or code out of range is the new R011.
- Parameters by reference: `procedimento troca(var a: inteiro, var b: inteiro)`. The subprogram changes the caller's own variable or vector element, as in `troca(x, y)` or `troca(v[i], v[i + 1])`. The argument must be a variable of exactly the same type.
- Warnings, shown in yellow as `aviso[W001]` with the same layout as errors. They point at things that compile but are probably mistakes: a variable never used (W001), read but never given a value (W002) or given a value that is never read (W003), the `para` variable changed inside its own loop (W004), a local variable that hides a global (W005), a function that can reach `fimfuncao` without `retorne` (W006), a division by a literal 0 (W007), a condition that is just `verdadeiro` or `falso` (W008) and commands right after `retorne` or `interrompa` (W009). Warnings appear only when the program has no errors, and the compilation still succeeds. `--no-warnings` hides them and `--warnings-as-errors` makes any warning stop the compilation with exit code 1. All the examples compile without warnings.
- `spike --explain E400` shows the explanation of an error code right in the terminal: the message, when it happens, how to fix it and an example. Lowercase works too (`e400`), and `spike --explain` alone lists every code. The text comes from `docs/errors.md`, copied into the executable at build time.
- Every compilation ends with a summary on stderr: `falhou: 2 erros e 1 aviso`, followed by `Para entender um erro: spike --explain E400` with the code of the first error. A successful build with warnings adds the count to the `ok:` line.
- `--color=auto|always|never`, accepted by every command. With `auto` (the default) colors appear only when the output is a terminal and `NO_COLOR` is not set; `FORCE_COLOR` turns them on without a terminal.
- A test (`error_docs_consistency`) that fails when an error code used in `src/` or `runtime/` has no entry in `docs/errors.md`, or when an entry is no longer used.
- Expressions are evaluated from left to right, also when a function in them changes a variable or reads input: `divide(17, 5, resto) + resto` uses the `resto` left by `divide`, and `le() - le()` subtracts the second value read from the first. The generated C keeps this order with temporaries (`spike_tmp_1`) only where it matters, since C itself leaves the order of arguments open.
- `spike fetch` shows the environment in a few lines, next to the ASCII dog: Spike's version, the system and architecture, the C compiler it found with the first line of its `--version`, whether `SPIKE_CC` is set and whether the temporary folder is writable. It ends with "tudo certo" or with what is missing, and exits with 3 when there is no C compiler. Paste it into an issue when asking for help.
- `spike dev programa.por` runs every phase and writes `programa.dev/` with `tokens.txt`, `ast.txt`, `symbols.txt`, `types.txt`, the generated `programa.c` and `times.txt`. When the program has errors it shows them and writes nothing.
- A recursion that never stops ends with runtime error R008 (more than 10000 calls in progress, or not enough memory left for calls) instead of crashing, and a function that reaches `fimfuncao` without `retorne` stops with the new runtime error R009.

- Runtime errors now look like compile errors: the code, ` --> arquivo.por:12`, the line of the program and a hint for each code. An error inside procedures or functions also lists where they were called, up to 8 lines (`chamado de 'fatorial' na linha 5`) and `... e mais N` for the rest. Colors only on a terminal, and never with `NO_COLOR` set.
- E903 (the C compiler rejected the generated code) now shows the `.por` line that produced the rejected C. The C that goes to the compiler carries `#line` directives for that; the copy saved by `--c` does not, so it stays easy to read.

### Changed

- Source files with Windows line endings or a UTF-8 byte order mark are read exactly like plain UTF-8 files, with the same line and column numbers. A file saved as Windows-1252 ("ANSI") is converted to UTF-8 and compiles, with the new warning W010 explaining how to save it as UTF-8.
- On Windows, Spike reads its command line as UTF-8 and switches the console input to UTF-8 too, so paths and folders with accents, spaces or letters outside the Windows code page work, including with `-o`.
- On macOS, E902 now suggests `xcode-select --install`.
- R008 for a recursion that never stops now points at the line of the call, not at the header of the subprogram.
- Generated procedures and functions take the line of the call as a first parameter (`f_fatorial(9, v_n)`), used by the call stack in runtime errors.
- Every push and pull request now builds and tests Spike on Linux (GCC and Clang), macOS and Windows (MSVC, with MinGW-w64 GCC compiling the generated C), plus a build with AddressSanitizer and UndefinedBehaviorSanitizer and a check that no source file passes 300 lines.
- `div`, `mod`, `const`, `interrompa`, `escolha`, `caso`, `outrocaso`, `fimescolha`, `vetor`, `procedimento`, `fimprocedimento`, `funcao`, `fimfuncao` and `retorne` are now reserved words. A program that used any of them as a variable name has to rename it.
- A procedure or function can no longer use the name of a built-in function such as `raiz` or `abs` (E789).
- On Windows, compiled programs get a 16 MB call stack, so deep recursion reaches the R008 limit before running out of stack.
- A command that starts with a name followed by `(`, like `imprima("oi")`, is now read as a procedure call. When no such procedure exists the error is E780 instead of E300.
- Syntax errors no longer pile up. After an error the parser skips to the next command and keeps checking, so independent mistakes are all reported and follow-up errors that were only a consequence of the first one are gone. A block left open is one error pointing at the word that opened it, and a missing value at the end of a line (`x <-`) no longer eats the next line. At most 20 errors are shown, followed by `(mais N erros omitidos)`. Absurdly deep nesting is the new error E202.
- Errors and warnings of the same phase are printed in line order.

### Fixed

- The arrow under an error no longer drifts right on lines with accented letters: columns now count characters, not bytes. Tabs are shown as 4 spaces and the arrow follows them.
- When something is missing at the end of a line, like the `entao` after `se x > 1`, the error points right after the last word of that line instead of the start of the next one.
- Lines longer than 100 characters are shown as an 80-character window around the error, with `...` at the cut ends.
- A file that is not valid UTF-8 (for example, saved as Windows-1252) gives the new error E005 at the first bad byte, instead of a pile of E001.
- A name with an accent, like `média`, gives one E001 with a hint that spells the name without accents, instead of one error per byte.
- Colors are no longer written when the output goes to a file or a pipe, so `spike programa.por 2> erros.txt` gives plain text.
- The `ok:` lines (`ok: gerado ...`, `ok: nenhum erro encontrado`) moved from stdout to stderr, next to the errors. With `--run`, stdout now has only the program's own output.
- Every entry of `docs/errors.md` follows the same format and has an example.
- `spike random` no longer pretends to work: it says the random program generator is planned for v1.0 and exits with 2. `spike --help` lists it under "planejados", apart from the commands that work.

## [0.2.0] - 2026-10-15

The first version that produces programs you can run. `spike programa.por` now goes all the way from Portugol to an executable.

### Added

- Compiling to an executable. Spike checks the program, turns it into C and calls GCC or Clang to build a standalone program. The compiler is looked up in `SPIKE_CC` first, then in `PATH`.
- Every command of the current language works in compiled programs: assignment, `escreva`, `escreval`, `leia`, `se`/`senao`, `enquanto`, `repita ... ate` and `para ... passo`.
- `leia` reads one value per line. Real numbers can be typed with a comma or a point, and a `logico` accepts `verdadeiro`, `falso`, `v`, `f`, `sim` and `nao`.
- Checks before compiling: variables used without being declared or declared twice, wrong types in expressions and assignments, conditions that are not `logico`, and `para` loops with a non-integer counter or a `passo 0` (errors E700 to E733).
- Runtime errors with the line of the `.por` file and exit code 101: division by zero (R001), invalid input (R003), a `para` whose step turns out to be zero (R006), input that ends too early (R007) and running out of memory (R008).
- Flags: `-o` to choose where the executable goes, `--run` to run it right after compiling, `--c` to keep the generated C, `--time` to see how long each step took, `--verbose` to follow each step and `--ast` to print the syntax tree.
- `spike check programa.por`, which only looks for errors and does not need a C compiler.
- Documentation: the language reference (`docs/language.md`), a catalog of every error with examples (`docs/errors.md`), the C runtime (`docs/runtime.md`) and the compiler internals (`docs/internals.md`).
- Example programs in `examples/` and end-to-end tests that compile real programs and compare their output.

### Changed

- Variable names ignore upper and lower case, like keywords: `Idade` and `idade` are the same variable.
- `/` always gives a real number: `7 / 2` is `3.5`.
- A real number with no fractional part is printed without it, as in VisuAlg: `7.0` prints `7`.
- `leia(1)` and other non-variables inside `leia` now report E306 instead of a placeholder code.

### Fixed

- Accented letters print correctly in the Windows console, in Spike's messages and in compiled programs.
- Input that starts with a UTF-8 byte order mark, as written by Notepad or piped from PowerShell, is read correctly.
- `-o` pointing to a folder that does not exist yet now creates the folder instead of failing.

[Unreleased]: https://github.com/joaopssx/spike-compiler/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/joaopssx/spike-compiler/releases/tag/v0.2.0
