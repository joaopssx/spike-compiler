# Spike Compiler

Basic project for a Portugol compiler.

Spike is a simple compiler made for study.
The first goal is small and clear: read Portugol code, generate C, and build a native program.

Portuguese version: `README.pt-BR.md`

## Goal

- Build a simple Portugol compiler.
- Generate C code as output.
- Use `gcc` or `clang` to create the executable.
- Keep the project small at the start.

## First roadmap idea

The project grows step by step:

- `0.x`: early development
- `1.0`: first stable version
- `1.x`: improvements and tools

## Initial pipeline

```text
source.por
  -> lexer
  -> parser
  -> AST
  -> C generator
  -> gcc/clang
  -> executable
```

## Version 0.1.0

Goal: read a `.por` file and recognize tokens.

Planned features:

- project structure
- basic CLI
- `.por` file reading
- simple lexer
- token list output
- `tokens` command

Example:

```bash
spike tokens file.por
```

Basic tokens for this version:

- keywords: `algoritmo`, `var`, `inicio`, `fimalgoritmo`, `inteiro`, `real`, `texto`, `leia`, `escreva`
- operators: `+`, `-`, `*`, `/`, `<-`, `=`, `<`, `>`, `<=`, `>=`

Main files for this version:

- `main`
- `lexer`
- `token`
- `token_type`

This version does not execute code yet.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/spike tokens file.por
```

Example:

```bash
./build/spike tokens examples/hello.por
```

## Notes

- No VM
- No LLVM
- No complex optimizations
- No IDE
- No website
- Only the essential parts first
