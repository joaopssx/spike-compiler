algoritmo "soma repita"
var
  i, soma : inteiro
inicio
  i <- 1
  repita
    soma <- soma + i
    i <- i + 1
  ate (i > 100) ou (soma < 0)
  escreval(soma)
fimalgoritmo
