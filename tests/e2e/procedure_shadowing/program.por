algoritmo "sombra"
var
  x, n : inteiro
procedimento dobra(n: inteiro)
var
  x : caractere
inicio
  x <- "local"
  n <- n * 2
  escreval(x, " ", n)
fimprocedimento
inicio
  x <- 1
  n <- 10
  dobra(n)
  escreval(x, " ", n)
fimalgoritmo
