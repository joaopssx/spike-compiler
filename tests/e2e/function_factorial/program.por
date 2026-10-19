// Fatorial de um número, calculado de dois jeitos: com recursão e com um laço.
algoritmo "fatorial"
var
  n : inteiro
funcao fatorial(n: inteiro): inteiro
inicio
  se n <= 1 entao
    retorne 1
  fimse
  retorne n * fatorial(n - 1)
fimfuncao
funcao fatorial_laco(n: inteiro): inteiro
var
  i, resultado : inteiro
inicio
  resultado <- 1
  para i de 2 ate n faca
    resultado <- resultado * i
  fimpara
  retorne resultado
fimfuncao
inicio
  escreva("Número: ")
  leia(n)
  escreval(n, "! = ", fatorial(n))
  escreval("Com laço: ", fatorial_laco(n))
fimalgoritmo
