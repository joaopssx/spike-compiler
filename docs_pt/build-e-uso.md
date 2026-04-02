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
./build/spike --help
./build/spike --version
./build/spike tokens caminho/do/arquivo.por
./build/spike tokens caminho/do/arquivo.por --verbose
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

Exemplo de saida detalhada:

```bash
./build/spike tokens examples/exemplo.por --verbose
```

```text
TOKEN_ALGORITMO lexeme="algoritmo" line=1 column=1
TOKEN_IDENTIFIER lexeme="soma" line=1 column=11
TOKEN_VAR lexeme="var" line=2 column=1
```

Exemplo de erro:

```text
Erro em tests/fixtures/invalid/invalid_escape.por: linha 3, coluna 14: sequencia de escape invalida: \q
    escreva("oi\q")
               ^
```

## Executando os testes

```bash
ctest --test-dir build --output-on-failure
```

## Erros atuais

Se o arquivo nao existir ou se o lexer encontrar um caractere invalido, o programa encerra com mensagem de erro.

Os erros lexicos agora incluem:

- nome do arquivo quando disponivel
- linha
- coluna
- a linha original do codigo
- um cursor `^` apontando a posicao do erro
