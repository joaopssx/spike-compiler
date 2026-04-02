# Build and usage

## Requirements

- CMake 3.16 or newer
- GCC or Clang with C++17 support

## Building

```bash
cmake -S . -B build
cmake --build build
```

The executable will be generated at `build/spike`.

## Available command

```bash
./build/spike tokens path/to/file.por
./build/spike tokens path/to/file.por --verbose
```

## Example

File `examples/exemplo.por`:

```text
algoritmo soma
var
  x, y: inteiro
  mensagem: texto
inicio
  leia(x)
  y <- 10
  escreva("resultado")
fimalgoritmo
```

Execution:

```bash
./build/spike tokens examples/exemplo.por
```

Expected output:

```text
TOKEN_ALGORITMO
TOKEN_IDENTIFIER
TOKEN_VAR
TOKEN_IDENTIFIER
TOKEN_COMMA
TOKEN_IDENTIFIER
TOKEN_COLON
TOKEN_INTEIRO
TOKEN_IDENTIFIER
TOKEN_COLON
TOKEN_TEXTO
TOKEN_INICIO
TOKEN_LEIA
TOKEN_LEFT_PAREN
TOKEN_IDENTIFIER
TOKEN_RIGHT_PAREN
TOKEN_IDENTIFIER
TOKEN_ASSIGN
TOKEN_NUMBER
TOKEN_ESCREVA
TOKEN_LEFT_PAREN
TOKEN_STRING
TOKEN_RIGHT_PAREN
TOKEN_FIMALGORITMO
```

Verbose output example:

```bash
./build/spike tokens examples/exemplo.por --verbose
```

```text
TOKEN_ALGORITMO lexeme="algoritmo" line=1 column=1
TOKEN_IDENTIFIER lexeme="soma" line=1 column=11
TOKEN_VAR lexeme="var" line=2 column=1
```

## Running tests

```bash
ctest --test-dir build --output-on-failure
```

## Current errors

If the file does not exist or the lexer finds an invalid character, the program exits with an error message.

Lexical errors now include:

- line
- column
- the source line
- a caret pointing to the error location
