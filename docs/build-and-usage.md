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

## Current errors

If the file does not exist or the lexer finds an invalid character, the program exits with an error message.
