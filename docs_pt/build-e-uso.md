# Build e uso

## Requisitos

- CMake 3.16 ou superior
- GCC ou Clang com suporte a C++17

## Compilando

```bash
cmake -S . -B build
cmake --build build
```

O executavel sera gerado em `build/spike`.

## Comando disponivel

```bash
./build/spike tokens caminho/do/arquivo.por
```

## Exemplo

Arquivo `examples/exemplo.por`:

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

Execucao:

```bash
./build/spike tokens examples/exemplo.por
```

Saida esperada:

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

## Erros atuais

Se o arquivo nao existir ou se o lexer encontrar um caractere invalido, o programa encerra com mensagem de erro.
