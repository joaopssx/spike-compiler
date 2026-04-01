# Spike

Spike is an educational Portugol compiler written in C++.

A Portuguese README is available in `readme_pt.md`.

At the moment, the project reads a `.por` file, runs lexical analysis, and prints the tokens it finds.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

```bash
./build/spike tokens examples/exemplo.por
```

## Documentation

English documentation is available in `docs/`.

Portuguese documentation is available in `docs_pt/`.
