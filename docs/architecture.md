# Spike Architecture

Spike is organized as a classic educational compiler pipeline.

## Pipeline

```text
Source Code
  -> Lexer
  -> Parser
  -> AST
  -> Semantic Analysis
  -> Code Generation
  -> C Source
  -> Native Executable
```

## Modules

### Lexer

Files: `include/token.hpp`, `include/lexer.hpp`, `src/lexer.cpp`

Responsibilities:

- read source code character by character
- emit tokens with line and column information
- recognize keywords, identifiers, numbers, strings and operators

### Parser

Files: `include/parser.hpp`, `src/parser.cpp`

Responsibilities:

- consume the token stream
- implement a recursive descent parser
- respect operator precedence
- construct the AST

### AST

Files: `include/ast.hpp`

Responsibilities:

- represent Portugol programs in memory
- store declarations, statements and expressions
- keep type and source location metadata for later stages

### Semantic Analysis

Files: `include/semantic.hpp`, `src/semantic.cpp`

Responsibilities:

- build the symbol table
- reject duplicate or undeclared variables
- infer expression types
- validate assignments and conditions

### Code Generation

Files: `include/codegen.hpp`, `src/codegen.cpp`

Responsibilities:

- walk the AST recursively
- emit valid C source code
- map Portugol types to C types
- preserve expression semantics with explicit parentheses

### CLI

File: `src/main.cpp`

Responsibilities:

- parse user commands and flags
- execute the pipeline stages in order
- print tokens or AST when requested
- optionally call the external C compiler

### Core Services

Files: `include/core/source_manager.hpp`, `include/core/diagnostics.hpp`, `include/core/session.hpp`, `include/core/phase_profiler.hpp`

Responsibilities:

- load the input file and expose source lines through `SourceManager`
- render better diagnostics through `DiagnosticEngine`
- centralize compilation state through `CompilationSession`
- measure each pipeline phase through `PhaseProfiler`

### CLI Layer

Files: `include/cli/options.hpp`, `src/cli/options.cpp`

Responsibilities:

- parse subcommands such as `build`, `run`, `tokens`, `ast`, `emit-c` and `trace`
- expose timing and tracing options like `--time` and `--trace`
- keep `main.cpp` focused on the pipeline instead of argument parsing details

## Design Goals

- easy to read for students learning compilers
- modern C++ memory safety through `std::unique_ptr`
- small, explicit code paths instead of hidden magic
- portable build through CMake
