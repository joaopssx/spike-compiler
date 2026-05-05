# Lexer

## What it does

The lexer reads the source code and splits the text into tokens.

Example:

```text
a <- "oi"
```

Output:

```text
IDENTIFIER (a)
ASSIGN (<-)
STRING ("oi")
```

## How tokens work

Each token represents one piece of the code, for example:

- identifiers: `name`, `value_1`
- numbers: `10`, `3.14`
- strings: `"text"`
- keywords: `algoritmo`, `inicio`, `fimalgoritmo`
- symbols and operators: `(`, `)`, `:`, `<-`, `<<`, `>>`, `{`, `}`

In this version, the lexer also recognizes `cout` and `cin` to prepare future styles.

Each token stores:

- type
- original value read from the file
- line
- column

## What the lexer ignores

In this version, the lexer ignores:

- spaces
- tabs
- blank lines
- `//` comments

## Note

The lexer only identifies tokens. The parser is still separate and was not changed in this step.
