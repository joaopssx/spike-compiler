# Spike Roadmap

Spike follows an incremental roadmap focused on shipping a real compiler before exploring advanced features.

## v0.1.0

- lexer
- recursive descent parser
- AST
- semantic analysis
- Portugol to C transpilation
- CLI with `tokens`, `ast`, `build`, `run` and `emit-c`

## v0.2.0

- stronger diagnostics with richer source snippets
- better support for `real`, `logico` and `caractere`
- more semantic checks and test coverage

## v0.3.0

- improved standard library surface for input/output
- formatted AST or JSON export mode
- GitHub release artifacts for Linux, macOS and Windows

## v1.0.0

- stable MVP language specification
- polished documentation and tutorials
- strong automated test suite
- production-ready CLI experience

## v2.0.0

- compiler-first repository focus with GitHub Linguist configured to prioritize C++ over website assets
- major C++ growth in `src/` and `include/`, targeting roughly 70% of the repository language stats as C++
- advanced compiler tooling: `--time`, `--trace`, `--stats`, `--ast-json`, `--graph`, `--doctor` and `--self-test`
- deeper semantic analysis and richer internal instrumentation for performance and debugging
- more automated tests centered on lexer, parser, semantic analysis and code generation
- website kept lightweight as documentation and project presentation, not as the core of the repository

## Future Ideas

- interpreter mode
- REPL
- formatter and linter
- VS Code extension
- LLVM backend
- WebAssembly target
