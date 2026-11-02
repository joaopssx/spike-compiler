# 7. Procedimentos e funções

Quando um trecho se repete, ou quando o programa fica grande, dá para dar um nome a um bloco de comandos. Procedimentos e funções ficam depois do `var` do programa e antes do `inicio` principal.

## Procedimentos

Um procedimento faz alguma coisa e não devolve valor:

```portugol-run
algoritmo "linhas"
procedimento linha(tamanho: inteiro)
var
  i : inteiro
inicio
  para i de 1 ate tamanho faca
    escreva("-")
  fimpara
  escreval()
fimprocedimento
inicio
  linha(10)
  escreval("Relatório")
  linha(10)
fimalgoritmo
```

```output
----------
Relatório
----------
```

`tamanho` é um parâmetro: o valor que quem chama passa entre parênteses. A variável `i` é local: só existe dentro do procedimento.

## Funções

Uma função calcula um valor e devolve com `retorne`. Na declaração, o tipo do resultado vem depois dos parênteses:

```portugol-run
algoritmo "fatorial"
funcao fatorial(n: inteiro): inteiro
inicio
  se n <= 1 entao
    retorne 1
  fimse
  retorne n * fatorial(n - 1)
fimfuncao
inicio
  escreval("5! = ", fatorial(5))
  escreval("10! = ", fatorial(10))
fimalgoritmo
```

```output
5! = 120
10! = 3628800
```

Essa função chama a si mesma (recursão). Toda recursão precisa de um caso de parada, aqui o `n <= 1`. Sem ele, o programa para com o erro R008 depois de 10000 chamadas.

## Parâmetros por referência

Normalmente o procedimento recebe uma cópia do valor. Com `var` antes do parâmetro, ele mexe na própria variável de quem chamou:

```portugol-run
algoritmo "troca"
var
  a, b : inteiro
procedimento troca(var x: inteiro, var y: inteiro)
var
  t : inteiro
inicio
  t <- x
  x <- y
  y <- t
fimprocedimento
inicio
  a <- 3
  b <- 8
  troca(a, b)
  escreval("a = ", a, ", b = ", b)
fimalgoritmo
```

```output
a = 8, b = 3
```

## Funções prontas

O Spike já traz várias funções, como `raiz`, `abs`, `arredonda`, `tamanho` e `maiusculo`. A lista completa está em [language.md](../language.md#built-in-functions).

```portugol-run
algoritmo "prontas"
inicio
  escreval(raiz(16))
  escreval(abs(-5))
  escreval(arredonda(2.5))
  escreval(tamanho("ação"))
  escreval(maiusculo("ação"))
fimalgoritmo
```

```output
4
5
3
4
AÇÃO
```

## Exercício

Escreva uma função `ehPrimo(n: inteiro): logico` e use para mostrar os números primos de 1 a 20.

## Solução

```portugol-run
algoritmo "primos"
var
  i : inteiro
funcao ehPrimo(n: inteiro): logico
var
  d : inteiro
inicio
  se n < 2 entao
    retorne falso
  fimse
  para d de 2 ate n - 1 faca
    se n mod d = 0 entao
      retorne falso
    fimse
  fimpara
  retorne verdadeiro
fimfuncao
inicio
  para i de 1 ate 20 faca
    se ehPrimo(i) entao
      escreva(i, " ")
    fimse
  fimpara
  escreval()
fimalgoritmo
```

```output
2 3 5 7 11 13 17 19
```
