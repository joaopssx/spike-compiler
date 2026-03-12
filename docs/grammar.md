# Spike Grammar

Spike currently implements a focused Portugol MVP designed for learning compiler construction.

## Grammar

```text
programa              -> "algoritmo" STRING bloco_var "inicio" lista_comandos "fimalgoritmo"

bloco_var             -> "var" lista_declaracoes | vazio
lista_declaracoes     -> declaracao lista_declaracoes | vazio
declaracao            -> lista_identificadores ":" tipo

lista_identificadores -> IDENTIFIER
                      | IDENTIFIER "," lista_identificadores

tipo                  -> "inteiro" | "real" | "logico" | "caractere"

lista_comandos        -> comando lista_comandos | vazio

comando               -> atribuicao
                      | escreva
                      | leia
                      | se
                      | enquanto

atribuicao            -> IDENTIFIER "<-" expressao
escreva               -> "escreva" "(" expressao ")"
leia                  -> "leia" "(" IDENTIFIER ")"

se                    -> "se" expressao "entao" lista_comandos bloco_senao "fimse"
bloco_senao           -> "senao" lista_comandos | vazio

enquanto              -> "enquanto" expressao "faca" lista_comandos "fimenquanto"

expressao             -> igualdade
igualdade             -> comparacao (("=" | "<>") comparacao)*
comparacao            -> termo (("<" | "<=" | ">" | ">=") termo)*
termo                 -> fator (("+" | "-") fator)*
fator                 -> unario (("*" | "/") unario)*
unario                -> "-" unario | primario
primario              -> NUMBER_INT | NUMBER_REAL | IDENTIFIER | "(" expressao ")"
```

## Tokens

Important token categories:

- keywords: `algoritmo`, `var`, `inicio`, `fimalgoritmo`, `inteiro`, `real`, `se`, `senao`, `entao`, `fimse`, `enquanto`, `faca`, `fimenquanto`, `escreva`, `leia`
- literals: `NUMBER_INT`, `NUMBER_REAL`, `STRING`
- operators: `<-`, `+`, `-`, `*`, `/`, `=`, `<>`, `<`, `<=`, `>`, `>=`
- punctuation: `(`, `)`, `:`, `,`

## Operator Precedence

From lowest to highest:

1. equality: `=` and `<>`
2. comparison: `<`, `<=`, `>`, `>=`
3. additive: `+`, `-`
4. multiplicative: `*`, `/`
5. unary: `-x`

## Semantic Rules

- a variable must be declared before use
- a variable cannot be declared twice in the same scope
- assignment to `inteiro` accepts only `inteiro`
- assignment to `real` accepts `inteiro` or `real`
- arithmetic operators require numeric operands
- relational operators require numeric operands and produce a logical result

## Notes

- the lexer and parser already recognize `logico` and `caractere`
- the current MVP backend is focused on `inteiro` and `real`
- string literals are currently reserved for the program name after `algoritmo`
