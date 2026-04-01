# Architecture

Spike was designed with a small and direct structure, focused on learning and gradual evolution.

## Current flow

At this stage, the flow is:

1. The CLI receives `spike tokens file.por`.
2. The file reading module loads the entire file into a `std::string`.
3. The `Lexer` walks through the text from left to right.
4. Each recognized piece becomes a `Token`.
5. The CLI prints the name of each token found.

## Separation of responsibilities

### `TokenType`

`TokenType` defines which token categories exist. It is an `enum class`, which makes the code safer and avoids mixing token types with raw integers.

### `Token`

`Token` stores:

- the token type
- the original lexeme
- line
- column

Even though the current CLI only prints the token name, line and column are already available for better error messages in future stages.

### `Lexer`

The `Lexer` receives the full source text and produces `std::vector<Token>`.

It was intentionally kept simple:

- one index for the current position
- line and column counters
- small functions for each kind of token reading

This approach makes the process easier to study without hiding the logic behind too many layers.

### `FileReader`

The file reading module was separated from the lexer so that I/O is not mixed with lexical analysis.

### `main.cpp`

`main.cpp` is responsible only for the command-line interface:

- validate arguments
- read the file
- call the lexer
- print the tokens

## Design decisions

1. The project uses only a few files, but each one has a clear role.
2. The lexer works in a single pass, which is the easiest way to understand and debug it at this stage.
3. The token stores the original lexeme because that will be useful in the parser and in error messages.
4. There is an internal end-of-file token, but it is not displayed by the `tokens` command to keep the output clean.
