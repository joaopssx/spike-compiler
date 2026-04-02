# Architecture

Spike was designed with a small and direct structure, focused on learning and gradual evolution.

## Current flow

At this stage, the flow is:

1. The CLI receives `spike tokens file.por`.
2. The file reading module loads the entire file into a `std::string`.
3. The `Lexer` walks through the text from left to right, optionally keeping the source file name for better diagnostics.
4. Each recognized piece becomes a `Token`.
5. The CLI prints either the token names or a verbose token view with lexeme, line, and column.

## Separation of responsibilities

### `TokenType`

`TokenType` defines which token categories exist. It is an `enum class`, which makes the code safer and avoids mixing token types with raw integers.

### `Token`

`Token` stores:

- the token type
- the original lexeme
- line
- column

The CLI can now print either the token name only or a verbose view with lexeme, line, and column.

Keyword recognition is case-insensitive, but the original source spelling is preserved in the token lexeme.

### `Lexer`

The `Lexer` receives the full source text and produces `std::vector<Token>`.

It was intentionally kept simple:

- one index for the current position
- line and column counters
- small functions for each kind of token reading
- one pass that skips whitespace and `//` comments before reading the next token
- validation for a small set of string escape sequences
- support for a broader token set used by control flow and logical expressions

This approach makes the process easier to study without hiding the logic behind too many layers.

### `FileReader`

The file reading module was separated from the lexer so that I/O is not mixed with lexical analysis.

### `main.cpp`

`main.cpp` is responsible only for the command-line interface:

- validate arguments
- read the file
- call the lexer
- print the tokens
- switch between normal and `--verbose` output
- handle `--help` and `--version`

### `tests/`

The project includes a small automated test executable for the lexer, plus `CTest` coverage for the CLI. The tests focus on the core behavior of this stage:

- token recognition
- comment skipping
- string escape handling
- error reporting
- fixture-based file inputs
- CLI behavior for help, version, and token printing

## Design decisions

1. The project uses only a few files, but each one has a clear role.
2. The lexer works in a single pass, which is the easiest way to understand and debug it at this stage.
3. The token stores the original lexeme because that will be useful in the parser and in error messages.
4. There is an internal end-of-file token, but it is not displayed by the `tokens` command to keep the output clean.
5. Automated tests were added now to keep future lexer changes safer without introducing a heavy test framework.
6. Escape handling remains intentionally small, covering only the sequences needed for this stage.
7. Keyword matching is case-insensitive to make educational Portugol inputs more natural.
