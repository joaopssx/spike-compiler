# Changelog

All notable changes to Spike are documented in this file.

## v2.0.0 (planned)

Compiler-first release planned to shift the repository weight back toward C++.

- GitHub Linguist configuration to keep website and docs from dominating language statistics
- larger C++ surface in compiler internals and tooling
- advanced developer flags such as timing, tracing, stats and AST export
- stronger test coverage around the compiler pipeline

## v0.1.0

Initial public structure for Spike.

- Modern C++ project setup with CMake
- Lexer, recursive descent parser and AST pipeline
- Semantic analysis for the MVP language subset
- Portugol to C code generation
- Native executable generation through the system C compiler
- Documentation, examples, website scaffold and CI workflow

Detailed release notes are available in `docs/changelog.md`.
