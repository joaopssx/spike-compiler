algoritmo "mesma variável"
var
  x : inteiro
procedimento troca(var a: inteiro, var b: inteiro)
var
  aux : inteiro
inicio
  aux <- a
  a <- b
  b <- aux
fimprocedimento
procedimento dobra_e_soma(var a: inteiro, var b: inteiro)
inicio
  a <- a * 2
  b <- b + 1
fimprocedimento
inicio
  x <- 5
  troca(x, x)
  escreval(x)
  dobra_e_soma(x, x)
  escreval(x)
fimalgoritmo
