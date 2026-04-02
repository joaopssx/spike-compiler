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

## Available commands

```bash
./build/spike --help
./build/spike --version
./build/spike tokens path/to/file.por
./build/spike tokens path/to/file.por --verbose
```

## Example

Available example files:

- `examples/exemplo.por`
- `examples/comentarios_e_strings.por`
- `examples/controle_fluxo.por`
- `examples/logica.por`

File `examples/logica.por`:

```text
algoritmo logica
var
  ativo: logico
inicio
  ativo <- verdadeiro
  se nao ativo ou ativo e falso entao
    escreva("falso")
  senao
    escreva("verdadeiro")
  fimse
fimalgoritmo
```

Execution:

```bash
./build/spike tokens examples/logica.por
```

Expected output:

```text
TOKEN_ALGORITMO
TOKEN_IDENTIFIER
TOKEN_VAR
TOKEN_IDENTIFIER
TOKEN_COLON
TOKEN_LOGICO
TOKEN_INICIO
TOKEN_IDENTIFIER
TOKEN_ASSIGN
TOKEN_VERDADEIRO
TOKEN_SE
TOKEN_NAO
TOKEN_IDENTIFIER
TOKEN_OU
TOKEN_IDENTIFIER
TOKEN_E
TOKEN_FALSO
TOKEN_ENTAO
TOKEN_ESCREVA
TOKEN_LEFT_PAREN
TOKEN_STRING
TOKEN_RIGHT_PAREN
TOKEN_SENAO
TOKEN_ESCREVA
TOKEN_LEFT_PAREN
TOKEN_STRING
TOKEN_RIGHT_PAREN
TOKEN_FIMSE
TOKEN_FIMALGORITMO
```

Verbose output example:

```bash
./build/spike tokens examples/controle_fluxo.por --verbose
```

```text
TOKEN_ALGORITMO lexeme="algoritmo" line=1 column=1
TOKEN_IDENTIFIER lexeme="fluxo" line=1 column=11
TOKEN_ENQUANTO lexeme="enquanto" line=6 column=3
```

Keywords are matched case-insensitively, so `algoritmo`, `Algoritmo`, and `ALGORITMO` all produce the same token type.

Error example:

```text
Erro em tests/fixtures/invalid/invalid_escape.por: linha 3, coluna 14: sequencia de escape invalida: \q
    escreva("oi\q")
               ^
```

## Running tests

```bash
ctest --test-dir build --output-on-failure
```

## Current errors

If the file does not exist or the lexer finds an invalid character, the program exits with an error message.

Lexical errors now include:

- file name when available
- line
- column
- the source line
- a caret pointing to the error location
