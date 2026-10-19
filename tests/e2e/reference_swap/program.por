// Troca o valor de duas variáveis com um procedimento de parâmetros por referência.
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
  escreval("antes: a = ", a, ", b = ", b)
  troca(a, b)
  escreval("depois: a = ", a, ", b = ", b)
fimalgoritmo
