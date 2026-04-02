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

## Comandos disponiveis

```bash
./build/spike --help
./build/spike --version
./build/spike tokens caminho/do/arquivo.por
./build/spike tokens caminho/do/arquivo.por --verbose
```

## Exemplo

Arquivos de exemplo disponiveis:

- `examples/exemplo.por`
- `examples/comentarios_e_strings.por`
- `examples/controle_fluxo.por`
- `examples/logica.por`

Arquivo `examples/logica.por`:

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

Execucao:

```bash
./build/spike tokens examples/logica.por
```

Saida esperada:

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

Exemplo de saida detalhada:

```bash
./build/spike tokens examples/controle_fluxo.por --verbose
```

```text
TOKEN_ALGORITMO lexeme="algoritmo" line=1 column=1
TOKEN_IDENTIFIER lexeme="fluxo" line=1 column=11
TOKEN_ENQUANTO lexeme="enquanto" line=6 column=3
```

As palavras-chave agora sao reconhecidas sem diferenca entre maiusculas e minusculas, entao `algoritmo`, `Algoritmo` e `ALGORITMO` viram o mesmo tipo de token.

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
