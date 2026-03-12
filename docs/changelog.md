# Detailed Changelog

## v2.0.0 (planned)

### Goals

- make Spike clearly compiler-first again in both repository structure and GitHub language breakdown
- expand the core C++ codebase substantially in `src/` and `include/`
- introduce advanced compiler instrumentation and debugging modes

### Planned

- GitHub Linguist tuning through `.gitattributes`
- timing, tracing and statistics flags
- AST JSON export and Graphviz export
- `doctor` and `self-test` commands
- more compiler-focused tests and internal utilities

## v0.1.0

Initial open-source project structure for Spike.

### Added

- lexer with token inspection mode
- recursive descent parser and AST dumping mode
- semantic analysis for declarations, assignments and conditions
- C code generator for the MVP Portugol subset
- automatic native build through the system C compiler
- CMake project, examples, docs, website scaffold and GitHub Actions workflow

### Notes

- the MVP is intentionally small and focused on educational clarity
- future versions will expand the language in controlled steps
