# Spike

Spike is a compiler for Portugol, the pseudo-language used to teach programming in Brazilian schools and universities. It reads a `.por` file, turns the program into C and builds a standalone executable that runs without Spike installed.

It is made for students in their first programming course. Error messages are in Portuguese, point at the exact spot in the code and come with a hint on how to fix it:

```
erro[E400]: 'entao' esperado após condição do 'se'
 --> media.por:6:5
     |
   5 |   se nota >= 7
   6 |     escreval("Aprovado")
     |     ^
   7 |   fimse
     |
     = dica: sintaxe: se <condicao> entao ... fimse.
```

## Status

Spike is early work. Today it reads and checks programs: the lexer and the parser are done, and the error messages above already work. It does not generate executables yet. The next milestone (v0.2) adds type checking, C generation and the first `.exe`.

The language follows the VisuAlg dialect. The full reference, including the syntax planned for vectors, functions and the rest, is in [docs/language.md](docs/language.md).

## A Portugol program

```
algoritmo "media"
var
  nota1, nota2, media : real
inicio
  escreva("Digite as duas notas: ")
  leia(nota1, nota2)
  media <- (nota1 + nota2) / 2
  se media >= 7 entao
    escreval("Aprovado com ", media)
  senao
    escreval("Reprovado com ", media)
  fimse
fimalgoritmo
```

More examples, one idea each, are in [examples/](examples/).

## Building

You need CMake 3.16 or newer and a C++17 compiler (GCC or Clang).

On Windows, the project is developed with MSYS2 UCRT64 and Ninja:

```
cmake -S . -B build -G Ninja
cmake --build build -j
```

On Linux and macOS, drop `-G Ninja`.

Run the tests with:

```
ctest --test-dir build --output-on-failure
```

On Windows, `C:\msys64\ucrt64\bin` has to come before any other MinGW folder in `PATH`. Git for Windows ships its own `mingw64\bin`, and if that one wins, the programs load the wrong `libstdc++-6.dll` and fail with `0xc0000139`.

## Usage

```
spike --help                  list commands and flags
spike --version               show the version
spike tokens programa.por     list the tokens and check that the program parses
```

`spike tokens` ends with `parse: OK` when the program is valid, or prints the errors found. `spike programa.por` is accepted but only runs the lexer for now; the compile flags (`-o`, `--run`, `--c`, `--ast`, `--time`, `--verbose`) are listed in the help and do nothing yet.

## Project layout

```
include/spike/   public headers
src/             one folder per module: lexer, parser, ast, diagnostics, cli, io
tests/           unit tests, one folder per module, plus programs/valid and programs/invalid
examples/        sample programs, also run as tests
docs/            language reference and product decisions
```

## License

MIT. See [LICENSE](LICENSE).
