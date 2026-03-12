# Spike

[![Build](https://img.shields.io/github/actions/workflow/status/joaopssx/spike/build.yml?branch=main&label=build)](https://github.com/joaopssx/spike/actions)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](CMakeLists.txt)

Spike is a modern Portugol compiler written in C++ that transpiles Portugol into C and generates native executables.

Created by Joao Pedro de Sena Santana. Named after the author's dog, Spike.

Portuguese readers can use `README.pt-BR.md` and the full translated documentation in `docs_pt/`.

## Features

- Modern C++ compiler architecture with `std::unique_ptr`, `std::vector` and `std::unordered_map`
- Lexer and recursive descent parser
- AST-based pipeline with semantic analysis
- Portugol to C transpilation
- Native executable generation through `gcc`, `clang` or compatible C compilers
- Debug-friendly CLI with token and AST inspection

## Supported MVP

- `algoritmo`, `var`, `inicio`, `fimalgoritmo`
- types `inteiro` and `real`
- assignment with `<-`
- `escreva(...)` and `leia(...)`
- `se / senao / fimse`
- `enquanto / faca / fimenquanto`
- arithmetic and relational operators

## Example

Portugol input:

```portugol
algoritmo "hello"

var
    a: inteiro

inicio
    a <- 10
    escreva(a)
fimalgoritmo
```

Run it with Spike:

```bash
./build/spike run examples/hello.por
```

Compiled output:

```text
10
```

## Project Layout

```text
spike/
├── include/
├── src/
├── examples/
├── docs/
├── docs_pt/
├── website/
├── tests/
├── scripts/
├── .github/workflows/
├── LICENSE
├── CHANGELOG.md
├── CMakeLists.txt
├── README.md
└── README.pt-BR.md
```

## Installation

Linux / macOS:

```bash
git clone https://github.com/joaopssx/spike
cd spike
cmake -S . -B build
cmake --build build
```

Windows:

```bash
git clone https://github.com/joaopssx/spike
cd spike
cmake -S . -B build
cmake --build build --config Release
```

## Usage

Default build flow:

```bash
./build/spike examples/hello.por
```

Professional subcommands:

```bash
./build/spike build examples/hello.por
./build/spike run examples/hello.por
./build/spike tokens examples/hello.por
./build/spike ast examples/if.por
./build/spike emit-c examples/loop.por --emit-c generated/loop.c
```

Flags:

```text
--tokens
--ast
--emit-c [arquivo.c]
--no-build
--run
--cc <compiler>
-o <binary>
```

## Documentation

- English docs live in `docs/`
- Portuguese docs live in `docs_pt/`
- `docs/grammar.md` / `docs_pt/grammar.md` - language grammar and precedence
- `docs/architecture.md` / `docs_pt/architecture.md` - compiler pipeline and module overview
- `docs/tutorial.md` / `docs_pt/tutorial.md` - quick start and examples
- `docs/roadmap.md` / `docs_pt/roadmap.md` - public roadmap
- `docs/contributing.md` / `docs_pt/contributing.md` - contribution guide
- `docs/changelog.md` / `docs_pt/changelog.md` - detailed release notes

## CI and Testing

- GitHub Actions workflow in `.github/workflows/build.yml`
- CMake/CTest smoke tests built into `CMakeLists.txt`
- Local test command:

```bash
ctest --test-dir build --output-on-failure
```

## Author

Joao Pedro de Sena Santana

## License

MIT License. See `LICENSE`.
