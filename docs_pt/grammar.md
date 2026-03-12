# Gramatica do Spike

Atualmente o Spike implementa um MVP enxuto de Portugol, pensado para estudar construcao de compiladores.

## Gramatica

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

Categorias importantes de tokens:

- palavras-chave: `algoritmo`, `var`, `inicio`, `fimalgoritmo`, `inteiro`, `real`, `se`, `senao`, `entao`, `fimse`, `enquanto`, `faca`, `fimenquanto`, `escreva`, `leia`
- literais: `NUMBER_INT`, `NUMBER_REAL`, `STRING`
- operadores: `<-`, `+`, `-`, `*`, `/`, `=`, `<>`, `<`, `<=`, `>`, `>=`
- pontuacao: `(`, `)`, `:`, `,`

## Precedencia de Operadores

Da menor para a maior:

1. igualdade: `=` e `<>`
2. comparacao: `<`, `<=`, `>`, `>=`
3. aditivos: `+`, `-`
4. multiplicativos: `*`, `/`
5. unario: `-x`

## Regras Semanticas

- toda variavel precisa ser declarada antes do uso
- uma variavel nao pode ser declarada duas vezes no mesmo escopo
- atribuicao para `inteiro` aceita apenas `inteiro`
- atribuicao para `real` aceita `inteiro` ou `real`
- operadores aritmeticos exigem operandos numericos
- operadores relacionais exigem operandos numericos e produzem resultado logico

## Observacoes

- o lexer e o parser ja reconhecem `logico` e `caractere`
- o backend atual do MVP esta focado em `inteiro` e `real`
- literais de string ainda estao reservados para o nome do programa apos `algoritmo`
