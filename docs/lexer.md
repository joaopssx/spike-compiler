# Lexer

## Goal

The lexer transforms the raw file text into a sequence of tokens.

Example:

```text
y <- 10
```

Becomes:

```text
TOKEN_IDENTIFIER
TOKEN_ASSIGN
TOKEN_NUMBER
```

## How it works

The `Lexer` reads one character at a time and decides which function to use based on the first character of the next token.

### 1. Whitespace and comments

Spaces, tabs, and line breaks are ignored.

Line comments starting with `//` are also ignored until the end of the line.

### 2. Identifiers and keywords

If the token starts with a letter or `_`, the lexer keeps reading letters, digits, and `_`.

After that, it checks whether the text is a known keyword. If it is, it returns the keyword token. Otherwise, it returns `TOKEN_IDENTIFIER`.

### 3. Numbers

If the token starts with a digit, the lexer reads a numeric sequence.

In this version, it accepts:

- integers: `10`
- simple decimals: `3.14`

### 4. Strings

Strings must use double quotes:

```text
"hello"
```

If the string is not closed, the lexer raises an error.

Supported escape sequences in this stage:

- `\"`
- `\\`
- `\n`
- `\t`

The lexer validates these escapes but still stores the original lexeme in the token.

### 5. Operators and symbols

Short operators and symbols are recognized directly from the current character, with special handling for two-character cases.

Examples:

- `<-`
- `<=`
- `>=`

## Supported tokens

### Keywords

- `algoritmo`
- `var`
- `inicio`
- `fimalgoritmo`
- `inteiro`
- `real`
- `texto`
- `leia`
- `escreva`

### Operators

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

### Symbols

- `:`
- `,`
- `(`
- `)`

### Generic token kinds

- `TOKEN_IDENTIFIER`
- `TOKEN_NUMBER`
- `TOKEN_STRING`

## Error messages

When the lexer finds an invalid character or an unterminated string, it reports:

- file name when available
- line
- column
- the original source line
- a caret showing the error position

## Intentional limitations of this stage

To keep the base simple and educational, this version does not yet include:

- parser
- AST
- C code generation

These features can be added later without making the current base more complex.
