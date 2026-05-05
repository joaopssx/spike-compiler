# Lexer

## O que ele faz

O lexer le o codigo fonte e quebra o texto em tokens.

Exemplo:

```text
a <- "oi"
```

Saida:

```text
IDENTIFIER (a)
ASSIGN (<-)
STRING ("oi")
```

## Como os tokens funcionam

Cada token representa uma parte do codigo, por exemplo:

- identificadores: `nome`, `valor_1`
- numeros: `10`, `3.14`
- strings: `"texto"`
- palavras-chave: `algoritmo`, `inicio`, `fimalgoritmo`
- simbolos e operadores: `(`, `)`, `:`, `<-`, `<<`, `>>`, `{`, `}`

Nesta versao, o lexer tambem reconhece `cout` e `cin` para preparar estilos futuros.

Cada token guarda:

- tipo
- valor original lido no arquivo
- linha
- coluna

## O que o lexer ignora

Nesta versao, o lexer ignora:

- espacos
- tabs
- linhas vazias
- comentarios com `//`

## Observacao

O lexer apenas identifica tokens. O parser continua separado e nao foi alterado nesta etapa.
