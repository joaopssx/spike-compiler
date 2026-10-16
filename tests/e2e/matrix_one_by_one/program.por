algoritmo "matriz 1x1"
var
  u : vetor[1..1, 1..1] de caractere
  r : vetor[5..5, 0..0, -1..-1] de real
inicio
  escreval("[", u[1, 1], "]")
  u[1, 1] <- "só"
  escreval("[", u[1, 1], "]")
  r[5, 0, -1] <- 2.5
  escreval(r[5, 0, -1] * 2)
fimalgoritmo
