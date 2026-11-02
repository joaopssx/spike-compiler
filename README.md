# Spike

[![CI](https://github.com/joaopssx/spike-compiler/actions/workflows/ci.yml/badge.svg)](https://github.com/joaopssx/spike-compiler/actions/workflows/ci.yml)

Spike compiles Portugol, the pseudo-language used to teach programming in Brazilian schools and universities, into a real executable. It is made for students in their first programming course.

## Why

- **It produces a program you can run anywhere.** `spike media.por` leaves a standalone executable next to your file. Send it to a friend; they do not need Spike to run it.
- **Errors are in Portuguese and point at the problem.** Every error shows the line, an arrow under the exact spot and a hint on how to fix it.
- **You can read what it generated.** Spike turns Portugol into C. With `--c` you keep that C file and can compare it with your program, which is a gentle first look at a "real" language.

## A first program

`media.por`:

```portugol-run
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

With `8` and `6,5` as input:

```input
8
6,5
```

the program prints

```output
Primeira nota: Segunda nota: Aprovado com média 7.25
```

On screen, each number you type shows up after its question. Real numbers can be typed with a comma or a point. Compiling it:

```
spike media.por
```

```
ok: gerado 'media.exe' (compilado em 0.4s)
```

Then run `media.exe` (or `./media` on Linux and macOS). It works on any computer, with or without Spike.

## What errors look like

A typo in a variable name (`notaa` instead of `nota1`):

```portugol-error E700
algoritmo "media"
var
  nota1, nota2 : real
inicio
  nota1 <- 7
  nota2 <- 8.5
  escreval("Média: ", (nota1 + nota2) / 2)
  escreval(notaa)
fimalgoritmo
```

```
erro[E700]: variável 'notaa' não foi declarada
 --> media.por:8:12
     |
   7 |   escreval("Média: ", (nota1 + nota2) / 2)
   8 |   escreval(notaa)
     |            ^
   9 | fimalgoritmo
     |
     = dica: você quis dizer 'nota1'? Declare 'notaa' na seção var.

falhou: 1 erro
Para entender um erro: spike --explain E700
```

Every error code is explained, with an example, in [docs/errors.md](docs/errors.md). The same text is built into Spike: `spike --explain E700` prints it in the terminal, and `spike --explain` lists every code.

## Documentation

For students, in Portuguese:

- [Primeiros passos](docs/getting-started.md): install, first program, reading an error.
- [Tutorial](docs/tutorial/README.md): nine short lessons with exercises.
- [Comandos e flags](docs/cli.md), [perguntas frequentes](docs/faq.md) and the [catálogo de erros](docs/errors.md).

Reference and internals, in English: [the language](docs/language.md), [the C runtime](docs/runtime.md), [how the compiler works](docs/internals.md) and [how to contribute](CONTRIBUTING.md).

## Downloading Spike

Ready-made packages are on the [Releases page](https://github.com/joaopssx/spike-compiler/releases): `spike-<version>-windows-x86_64.zip`, `spike-<version>-linux-x86_64.tar.gz`, `spike-<version>-macos-arm64.tar.gz` (Apple Silicon) and `spike-<version>-macos-x86_64.tar.gz` (Intel Macs). Each one holds `spike`, the license, this README, the changelog and the `examples/` folder. `SHA256SUMS.txt` lists the checksum of every package.

**Windows**

1. Download the `.zip`, right-click it, choose "Extract All" and move the folder somewhere permanent, such as `C:\Spike`.
2. Add that folder to your `PATH` (Start menu, "Edit environment variables for your account", `Path`, New).
3. Open a new terminal and run `spike setup` once. It downloads the C compiler Spike needs (about 64 MB) and checks it before using it.
4. Check the download, if you like: `Get-FileHash spike-<version>-windows-x86_64.zip` in PowerShell must print the hash listed in `SHA256SUMS.txt`.

**Linux and macOS**

```
tar -xzf spike-<version>-linux-x86_64.tar.gz
sudo cp spike-<version>-linux-x86_64/spike /usr/local/bin/
spike setup
```

`spike setup` tells you whether a C compiler is already installed and, if not, prints the command for your system (`xcode-select --install` on macOS). To check the download, compare `sha256sum <package>` (Linux) or `shasum -a 256 <package>` (macOS) with the line in `SHA256SUMS.txt`. macOS may block a downloaded `spike` the first time; [docs/faq.md](docs/faq.md) explains how to allow it.

The Linux package runs on distributions with glibc 2.35 or newer (Ubuntu 22.04, Debian 12 and later).

## Installing from source

You can also build Spike yourself. You need a C compiler (GCC or Clang) installed, because Spike uses it to produce the executable.

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
| `spike setup` | Installs a C compiler (Windows) or prints the command that installs one (Linux, macOS). `--check`, `--force`, `--remove`, `--yes`. |
| `spike --explain E700` | Explains an error code; without a code, lists them all. |
| `spike --help`, `spike --version` | Help and version. |

Flags can be combined: `spike programa.por --c --time -o saida/prog`. `--color=auto|always|never` works with every command. Every flag, exit code and environment variable is in [docs/cli.md](docs/cli.md).

`spike random`, a generator of random Portugol programs, is planned for v1.0. Today it only says so and exits with 2.

Exit codes: 0 when everything worked, 1 when the program has errors, 2 when the command line is wrong, 3 when the C compiler is missing or fails. A compiled program that stops on a runtime error, such as a division by zero, exits with 101.

## The language

Spike follows the VisuAlg dialect. The full reference is [docs/language.md](docs/language.md), with a section on where Spike differs from VisuAlg.

- Types `inteiro` (64 bits), `real`, `caractere` and `logico`; constants; vectors and matrices up to 3 dimensions.
- Arithmetic with `+ - * /`, `div`, `mod` and `^`; comparisons; `e`, `ou`, `nao`; joining texts with `+`. `/` always gives a real number.
- `escreva`, `escreval`, `leia`, `se`/`senao`, `escolha`, `enquanto`, `repita ... ate`, `para ... passo` and `interrompa`.
- Procedures and functions, with parameters by value or by reference (`var`) and recursion.
- 28 built-in functions for math and text, such as `raiz`, `arredonda`, `aleatorio`, `tamanho` and `maiusculo`.
- Names ignore upper and lower case.

## Status

**v0.4.** The whole language above compiles to a standalone executable on Windows, Linux and macOS. Spike checks names and types before generating code, explains every error in Portuguese with a hint, warns about likely mistakes, and a compiled program stops with a clear message (and the line of the `.por`) on runtime errors such as division by zero or an index outside a vector. `spike setup` installs the C compiler on Windows, and every tagged version is built, tested and published for the three systems.

What comes next:

- **v0.5:** a VS Code extension, a formatter and fuzzing.
- **v1.0:** an interpreter and a REPL, a playground in the browser and a language server.

## Contributing

Issues and pull requests are welcome at [github.com/joaopssx/spike-compiler](https://github.com/joaopssx/spike-compiler/issues). [CONTRIBUTING.md](CONTRIBUTING.md) explains how to build, run the tests, the code style and how to report a bug.

```
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

On Windows with MSYS2, add `-G Ninja` to the first command.

## License

MIT. See [LICENSE](LICENSE).
