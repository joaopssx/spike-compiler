// Os 10 primeiros números de Fibonacci, cada um calculado por uma função recursiva.
algoritmo "fibonacci"
var
  i : inteiro
funcao fib(n: inteiro): inteiro
inicio
  se n < 2 entao
    retorne n
  fimse
  retorne fib(n - 1) + fib(n - 2)
fimfuncao
inicio
  escreva(fib(0))
  para i de 1 ate 9 faca
    escreva(", ", fib(i))
  fimpara
  escreval()
fimalgoritmo
