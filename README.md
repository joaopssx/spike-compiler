# Spike

[![CI](https://github.com/joaopssx/spike-compiler/actions/workflows/ci.yml/badge.svg)](https://github.com/joaopssx/spike-compiler/actions/workflows/ci.yml)

Spike compiles Portugol, the pseudo-language used to teach programming in Brazilian schools and universities, into a real executable. It is made for students in their first programming course.

## Why

- **It produces a program you can run anywhere.** `spike media.por` leaves a standalone executable next to your file. Send it to a friend; they do not need Spike to run it.
- **Errors are in Portuguese and point at the problem.** Every error shows the line, an arrow under the exact spot and a hint on how to fix it.
- **You can read what it generated.** Spike turns Portugol into C. With `--c` you keep that C file and can compare it with your program, which is a gentle first look at a "real" language.

## A first program

`media.por`:

```
algoritmo "media"
var
  nota1, nota2, media : real
inicio
  escreva("Primeira nota: ")
  leia(nota1)
  escreva("Segunda nota: ")
  leia(nota2)
  media <- (nota1 + nota2) / 2
  se media >= 7 entao
    escreval("Aprovado com média ", media)
  senao
    escreval("Reprovado com média ", media)
  fimse
fimalgoritmo
```

```
$ spike media.por
ok: gerado 'media' (compilado em 0.4s)
$ ./media
Primeira nota: 8
Segunda nota: 6,5
Aprovado com média 7.25
```

Real numbers can be typed with a comma or a point. On Windows the executable is `media.exe`.

## What errors look like

A typo in a variable name (`meida` instead of `media`):

```
$ spike erro.por
erro[E700]: variável 'meida' não foi declarada
 --> erro.por:7:24
     |
   6 |   media <- (nota1 + nota2) / 2
   7 |   escreval("Média: ", meida)
     |                        ^
   8 | fimalgoritmo
     |
     = dica: declare 'meida' na seção var.

falhou: 1 erro
Para entender um erro: spike --explain E700
```

Every error code is explained, with an example, in [docs/errors.md](docs/errors.md). The same text is built into Spike: `spike --explain E700` prints it in the terminal, and `spike --explain` lists every code.

## Installing

There are no installers yet. You build Spike from source, and you need a C compiler (GCC or Clang) installed, because Spike uses it to produce the executable.

**Debian / Ubuntu**

```
sudo apt install build-essential cmake git
git clone https://github.com/joaopssx/spike-compiler.git
cd spike-compiler
cmake -S . -B build
cmake --build build -j
```

**macOS**

```
xcode-select --install
```

That installs Clang. Install CMake from [cmake.org](https://cmake.org/download/) or with `brew install cmake`, then run the same `git clone` and `cmake` commands as above.

**Windows**

1. Install [MSYS2](https://www.msys2.org/).
2. Open the "MSYS2 UCRT64" terminal and install the tools:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja git
   ```
3. In the same terminal, clone and build:
   ```
   git clone https://github.com/joaopssx/spike-compiler.git
   cd spike-compiler
   cmake -S . -B build -G Ninja
   cmake --build build -j
   ```

To use Spike from PowerShell or `cmd`, add `C:\msys64\ucrt64\bin` to your `PATH`, before any other MinGW folder. Git for Windows ships its own `mingw64\bin`; if that one comes first, programs fail to start with error `0xc0000139`.

To compile Portugol programs, Spike also needs a C compiler. On Windows, `spike setup` downloads a portable GCC (w64devkit, about 64 MB) into `%LOCALAPPDATA%\Spike`, checks its SHA-256 before using it, and Spike finds it from then on. On Linux and macOS, `spike setup` prints the command to install one. See [docs/getting-started.md](docs/getting-started.md).

The compiler ends up in `build/spike` (`build\spike.exe` on Windows). Run it from there or copy it to a folder in your `PATH`.

## Usage

| Command | What it does |
| --- | --- |
| `spike programa.por` | Compiles to an executable next to the file (`programa`, or `programa.exe` on Windows). |
| `spike programa.por -o caminho` | Writes the executable to `caminho`. |
| `spike programa.por --run` | Compiles and runs the program right away. Spike exits with the program's exit code. |
| `spike programa.por --c` | Also keeps the generated C, as `programa.c`. |
| `spike programa.por --time` | Shows how long each step took. |
| `spike programa.por --verbose` | Describes each step as it happens. |
| `spike programa.por --ast` | Prints the program's syntax tree and does not compile. |
| `spike programa.por --no-warnings` | Hides the warnings (`aviso[W001]` and friends). |
| `spike programa.por --warnings-as-errors` | Stops the compilation, with exit code 1, when there is any warning. |
| `spike check programa.por` | Only checks the program for errors. Does not need a C compiler. |
| `spike tokens programa.por` | Lists the tokens of the file (for debugging). |
| `spike dev programa.por` | Runs every phase and writes the tokens, syntax tree, symbol table, expression types, generated C and phase times to `programa.dev/`. Nothing is written when the program has errors. |
| `spike fetch` | Shows Spike's version, the system, the C compiler it found and whether the temporary folder is writable. Paste its output when asking for help. Exits with 3 when something is missing. |
| `spike --help`, `spike --version` | Help and version. |

Flags can be combined: `spike programa.por --c --time -o saida/prog`.

`spike random`, a generator of random Portugol programs, is planned for v1.0. Today it only says so and exits with 2.

Exit codes: 0 when everything worked, 1 when the program has errors, 2 when the command line is wrong, 3 when the C compiler is missing or fails. A compiled program that stops on a runtime error, such as a division by zero, exits with 101.

## The language

Spike follows the VisuAlg dialect. The full reference is [docs/language.md](docs/language.md).

What works today: the types `inteiro`, `real`, `caractere` and `logico`; variables; arithmetic, comparisons, `e`, `ou`, `nao` and joining texts with `+`; `escreva`, `escreval` and `leia`; `se`/`senao`, `enquanto`, `repita ... ate` and `para ... passo`. Names ignore upper and lower case. `/` always gives a real number.

Planned next, with the syntax already proposed in the reference: `div`, `mod` and `^`, constants, `interrompa`, `escolha`, vectors and matrices, procedures and functions, and built-in functions such as `raiz`, `abs` and `tamanho`.

## Status

**v0.2.** Spike compiles programs with variables, expressions, `se`, `enquanto`, `repita`, `para`, `leia` and `escreva`. It checks names and types before generating code, and a compiled program stops with a clear message on division by zero, invalid input or a `para` with step zero.

What comes next:

- **v0.3:** the rest of the language listed above, warnings, and better recovery after the first error.
- **v0.4:** a C compiler bundled with Spike, installers for Windows, Linux and macOS, and automated builds on all three.
- **Later:** a VS Code extension, a formatter and a playground in the browser.

## Contributing

Issues and pull requests are welcome at [github.com/joaopssx/spike-compiler](https://github.com/joaopssx/spike-compiler/issues).

Build and run the tests:

```
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

On Windows with MSYS2, add `-G Ninja` to the first command. `ctest --test-dir build -L e2e` runs only the end-to-end tests, which compile real programs and compare their output with the expected one. To add one, see [tests/e2e/README.md](tests/e2e/README.md). How the compiler works inside is in [docs/internals.md](docs/internals.md).

## License

MIT. See [LICENSE](LICENSE).
