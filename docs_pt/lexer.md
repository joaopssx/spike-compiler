# Lexer

## Objetivo

O lexer transforma o texto bruto do arquivo em uma sequencia de tokens.

Exemplo:

```text
y <- 10
```

Vira:

```text
TOKEN_IDENTIFIER
TOKEN_ASSIGN
TOKEN_NUMBER
```

## Como ele funciona

O `Lexer` le um caractere por vez e decide qual funcao usar com base no primeiro caractere do proximo token.

### 1. Espacos em branco e comentarios

Espacos, tabulacoes e quebras de linha sao ignorados.

Comentarios de linha iniciados com `//` tambem sao ignorados ate o fim da linha.

### 2. Identificadores e palavras-chave

Se o token comeca com letra ou `_`, o lexer continua lendo letras, digitos e `_`.

Depois disso ele verifica se o texto lido e uma palavra-chave conhecida. Se for, retorna o token da palavra-chave. Se nao for, retorna `TOKEN_IDENTIFIER`.

### 3. Numeros

Se o token comeca com digito, o lexer le uma sequencia numerica.

Nesta versao ele aceita:

- inteiros: `10`
- decimais simples: `3.14`

### 4. Strings

Strings devem usar aspas duplas:

```text
"ola"
```

Se a string nao for fechada, o lexer gera erro.

### 5. Operadores e simbolos

Operadores e simbolos curtos sao reconhecidos diretamente pelo caractere atual, com tratamento especial para os casos de dois caracteres.

Exemplos:

- `<-`
- `<=`
- `>=`

## Tokens suportados

### Palavras-chave

- `algoritmo`
- `var`
- `inicio`
- `fimalgoritmo`
- `inteiro`
- `real`
- `texto`
- `leia`
- `escreva`

### Operadores

- `+`
- `-`
- `*`
- `/`
- `<-`
- `=`
- `<`
- `>`
- `<=`
- `>=`

### Simbolos

- `:`
- `,`
- `(`
- `)`

### Tipos genericos

- `TOKEN_IDENTIFIER`
- `TOKEN_NUMBER`
- `TOKEN_STRING`

## Mensagens de erro

Quando o lexer encontra um caractere invalido ou uma string nao terminada, ele informa:

- linha
- coluna
- a linha original do codigo
- um cursor mostrando a posicao do erro

## Limitacoes intencionais desta fase

Para manter a base simples e didatica, esta versao ainda nao faz:

- parser
- AST
- geracao de codigo C
- tratamento de escapes em strings

Esses pontos podem entrar depois, sem complicar a base atual.
