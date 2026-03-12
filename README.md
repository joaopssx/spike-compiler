# Spike

[![Build](https://img.shields.io/github/actions/workflow/status/joaopssx/spike/build.yml?branch=main&label=build)](https://github.com/joaopssx/spike/actions)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](CMakeLists.txt)
[![Website](https://img.shields.io/badge/site-GitHub%20Pages-2ea44f)](https://joaopssx.github.io/spike/)

Spike is a modern Portugol compiler written in C++ that transpiles Portugol into C and generates native executables.

Created by Joao Pedro de Sena Santana. Named after the author's dog, Spike.

Live website: `https://joaopssx.github.io/spike/`

Portuguese readers can use `README.pt-BR.md` and the full translated documentation in `docs_pt/`.

## Features

- Modern C++ compiler architecture with `std::unique_ptr`, `std::vector` and `std::unordered_map`
- Compiler core split into `cli`, `core`, lexer, parser, semantic analysis and backend-oriented modules
- Lexer and recursive descent parser
- AST-based pipeline with semantic analysis
- Portugol to C transpilation
- Native executable generation through `gcc`, `clang` or compatible C compilers
- Debug-friendly CLI with token and AST inspection
- Phase timing and trace output with `--time` and `--trace`
- Structured outputs with `spike stats`, `spike explain`, `spike ast --json` and `spike ast --graph`
- Initial IR layer with `spike ir` and a first optimization pass for constant folding

## Repository Shape

- serious compiler-first repository structure
- strong English and Portuguese documentation under `docs/en/` and `docs/pt/`
- issue templates, CI workflows and contribution files for GitHub
- room for IR, passes, formatter, linter and watch mode without turning the codebase into a mess

## v2.0.0 Direction

- Spike v2.0.0 is planned as a compiler-first release
- the repository is being prepared so GitHub language stats reflect the real project focus: C++
- website and documentation assets are marked as documentation in `.gitattributes`, while future work expands the C++ core significantly
- the target direction is a much stronger C++ share, around 70% of the repository language breakdown

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
│   ├── cli/
│   ├── core/
│   └── ...
├── src/
│   ├── cli/
│   ├── core/
│   └── ...
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
./build/spike check examples/hello.por
./build/spike trace examples/hello.por
./build/spike tokens examples/hello.por
./build/spike ast examples/if.por
./build/spike ast examples/if.por --json --output generated/if.ast.json
./build/spike ast examples/if.por --graph --output generated/if.ast.dot
./build/spike ir examples/basic/hello.por
./build/spike ir examples/basic/hello.por --json --output generated/hello.ir.json
./build/spike stats examples/basic/hello.por
./build/spike explain examples/basic/hello.por
./build/spike emit-c examples/loop.por --emit-c generated/loop.c
./build/spike doctor
./build/spike version
```

Flags:

```text
--tokens
--ast
--emit-c [arquivo.c]
--no-build
--run
--json
--graph
--output <arquivo>
--stdout
--with-spans
--trace
--time
--cc <compiler>
-o <binary>
```

## Documentation

- English docs live in `docs/en/`
- Portuguese docs live in `docs/pt/`
- `docs/en/cli.md` / `docs/pt/cli.md` - command reference
- `docs/en/architecture.md` / `docs/pt/architecture.md` - compiler pipeline and internal modules
- `docs/en/ir.md` / `docs/pt/ir.md` - intermediate representation layer
- `docs/en/tutorial.md` / `docs/pt/tutorial.md` - guided usage
- `docs/en/roadmap.md` / `docs/pt/roadmap.md` - roadmap from `v0.1.0` to `v1.0.0`
- `docs/en/contributing.md` / `docs/pt/contributing.md` - contribution guide
- `docs/index.md` - documentation index

## Website and GitHub Pages

- The static site lives in `website/`
- Automatic deployment is configured in `.github/workflows/pages.yml`
- After enabling GitHub Pages with GitHub Actions, the URL is `https://joaopssx.github.io/spike/`
- The website is automatically updated on every push to `main`
- English is the default website language, with a header button to switch to Portuguese

Main pages:

- `website/index.html` - landing page
- `website/docs.html` - documentation overview
- `website/tutorial.html` - step-by-step tutorial

## CI and Testing

- GitHub Actions workflow in `.github/workflows/build.yml`
- GitHub Pages deployment workflow in `.github/workflows/pages.yml`
- CMake/CTest smoke tests built into `CMakeLists.txt`
- Local test command:

```bash
ctest --test-dir build --output-on-failure
```

## Author

Joao Pedro de Sena Santana

## License

MIT License. See `LICENSE`.
