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

## Design Goals

- easy to read for students learning compilers
- modern C++ memory safety through `std::unique_ptr`
- small, explicit code paths instead of hidden magic
- portable build through CMake
