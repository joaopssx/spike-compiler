# Roadmap

## v0.1.0 - Foundation

- project layout and CMake baseline
- source loading, lexer, parser and AST foundation
- basic semantic analysis and C code generation
- `spike build` and `spike run`

## v0.2.0 - Control Flow

- `se`, `senao`, `fimse`
- `enquanto`, `faca`, `fimenquanto`
- better expression precedence
- `spike tokens` and `spike ast`

## v0.3.0 - Semantic Analysis

- symbol table improvements
- duplicate and undeclared variable checks
- assignment and condition type validation
- `spike check`

## v0.4.0 - Better Tooling

- `spike emit-c`
- richer AST inspection
- JSON and Graphviz export scaffolding
- clearer docs in English and Portuguese

## v0.5.0 - Diagnostics Upgrade

- structured diagnostic engine
- better source spans
- warning, note and help output
- parser recovery improvements

## v0.6.0 - IR Layer

- initial intermediate representation
- IR builder and printer
- cleaner separation between frontend and backend

## v0.7.0 - Optimization Passes

- pass manager
- constant folding
- constant propagation
- dead code analysis groundwork

## v0.8.0 - Profiling and Advanced CLI

- `--time`
- `--mem`
- `--leaks`
- `spike doctor`
- `spike benchmark`

## v0.9.0 - Docs, Site and Cross-Platform Polish

- stronger docs PT/EN
- polished website and screenshots
- CI on Linux, Windows and macOS
- release preparation

## v0.9.5 - Ecosystem Tools

- formatter
- linter
- watch mode
- improved explain mode

## v1.0.0 - Stable Core Release

- stable frontend, semantic analysis and C backend
- solid CLI and diagnostics
- complete documentation and examples
- releases ready for public showcase and portfolio use
