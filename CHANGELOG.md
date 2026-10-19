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
- A recursion that never stops ends with runtime error R008 (more than 10000 calls in progress, or not enough memory left for calls) instead of crashing, and a function that reaches `fimfuncao` without `retorne` stops with the new runtime error R009.

### Changed

- `div`, `mod`, `const`, `interrompa`, `escolha`, `caso`, `outrocaso`, `fimescolha`, `vetor`, `procedimento`, `fimprocedimento`, `funcao`, `fimfuncao` and `retorne` are now reserved words. A program that used any of them as a variable name has to rename it.
- On Windows, compiled programs get a 16 MB call stack, so deep recursion reaches the R008 limit before running out of stack.
- A command that starts with a name followed by `(`, like `imprima("oi")`, is now read as a procedure call. When no such procedure exists the error is E780 instead of E300.

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
