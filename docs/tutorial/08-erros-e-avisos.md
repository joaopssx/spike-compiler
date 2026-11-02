# 8. Erros e avisos

Errar faz parte. O Spike tenta dizer onde está o problema e como resolver, sempre em português.

## Lendo um erro

Este programa esqueceu o `entao`:

```portugol-error E400
algoritmo "erro"
var
  x : inteiro
inicio
  se x > 0
    escreval("positivo")
  fimse
fimalgoritmo
```

O Spike responde:

```
erro[E400]: 'entao' esperado após condição do 'se'
 --> se.por:5:11
     |
   4 | inicio
   5 |   se x > 0
     |           ^
   6 |     escreval("positivo")
     |
     = dica: sintaxe: se <condicao> entao ... fimse.

falhou: 1 erro
Para entender um erro: spike --explain E400
```

- `erro[E400]` é o código do erro. Cada problema tem o seu.
- `se.por:5:11` é o arquivo, a linha 5 e a coluna 11.
- O trecho do meio mostra a linha com o problema, com as vizinhas para dar contexto. A seta `^` aponta o lugar exato: aqui, o ponto onde o `entao` deveria estar.
- `dica:` sugere como corrigir.

Quando há vários erros, corrija o primeiro e compile de novo: às vezes um erro só provoca os outros.

## Mais explicação: --explain

`spike --explain` seguido do código mostra a explicação completa, com um exemplo:

```
spike --explain E400
```

```
E400 — falta 'entao'

Mensagem: `'entao' esperado após condição do 'se'`

Quando acontece: a condição do `se` não foi seguida de `entao`.

Como corrigir: escreva `se condição entao`.
```

(A saída continua com um programa de exemplo.) `spike --explain` sem código lista todos os erros. O catálogo também está em [errors.md](../errors.md).

## Sugestões

Quando um nome parece com outro que existe, o Spike sugere a correção:

```portugol-error E700
algoritmo "media"
var
  nota1, nota2 : real
inicio
  nota1 <- 7
  nota2 <- 8.5
  escreval(notaa)
fimalgoritmo
```

A dica desse erro é `você quis dizer 'nota1'? Declare 'notaa' na seção var.`. O mesmo vale para comandos (`escrva` sugere `escreva`) e para coisas de outras linguagens: `print` sugere `escreva()`, e `x == 1` lembra que em Portugol a igualdade usa um `=` só.

## Avisos

Um aviso não impede a compilação. Ele aponta algo que provavelmente é engano:

```portugol
algoritmo "aviso"
var
  x, sobra : inteiro
inicio
  x <- 1
  escreval(x)
fimalgoritmo
```

```
aviso[W001]: a variável 'sobra' foi declarada, mas nunca é usada
 --> aviso.por:3:6
     |
   2 | var
   3 |   x, sobra : inteiro
     |      ^
   4 | inicio
     |
     = dica: apague a declaração ou use a variável no programa.

ok: gerado 'aviso.exe' (compilado em 0.4s), 0 erros, 1 aviso
```

`--no-warnings` esconde os avisos, e `--warnings-as-errors` faz qualquer aviso parar a compilação.

## Erros de execução

Alguns problemas só aparecem com o programa rodando, como uma divisão por zero com valores lidos do teclado. O programa para e mostra uma mensagem no mesmo formato, com código `R`:

```
erro de execução[R001]: divisão por zero
 --> divisao.por:7
     |
   7 |   escreval(a div b)
     |
     = dica: verifique se o divisor pode valer 0 antes de dividir (use um 'se')
```

## Exercício

O programa abaixo tem dois erros. Descubra quais são, primeiro lendo, depois com o Spike.

```portugol-error E700
algoritmo "conta"
var
  total : inteiro
inicio
  total <- 10
  se totl > 5 entao
    escreval("grande")
  fimse
  escreval(total / 2)
  total <- total / 2
fimalgoritmo
```

## Solução

`totl` está escrito errado (E700, com a sugestão `total`) e `total / 2` dá um `real`, que não pode ir para um `inteiro` (E721). Use `div`:

```portugol-run
algoritmo "conta"
var
  total : inteiro
inicio
  total <- 10
  se total > 5 entao
    escreval("grande")
  fimse
  escreval(total / 2)
  total <- total div 2
fimalgoritmo
```

```output
grande
5
```
