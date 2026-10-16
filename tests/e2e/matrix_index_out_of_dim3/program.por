algoritmo "fora da dimensão 3"
var
  c : vetor[1..2, 1..2, 0..1] de real
  k : inteiro
inicio
  k <- 2
  c[1, 2, k - 1] <- 1.5
  escreval(c[1, 2, 1])
  c[2, 1, k] <- 3
fimalgoritmo
