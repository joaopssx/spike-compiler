// Ordenação bolha de 10 números, com um procedimento que troca dois elementos do vetor.
algoritmo "ordenação bolha"
const
  N = 10
var
  v : vetor[1..N] de inteiro
  i, j : inteiro
procedimento troca(var x: inteiro, var y: inteiro)
var
  t : inteiro
inicio
  t <- x
  x <- y
  y <- t
fimprocedimento
inicio
  v[1] <- 42
  v[2] <- 7
  v[3] <- 19
  v[4] <- -3
  v[5] <- 88
  v[6] <- 0
  v[7] <- 15
  v[8] <- 7
  v[9] <- 61
  v[10] <- 23
  para i de 1 ate N - 1 faca
    para j de 1 ate N - i faca
      se v[j] > v[j + 1] entao
        troca(v[j], v[j + 1])
      fimse
    fimpara
  fimpara
  escreva(v[1])
  para i de 2 ate N faca
    escreva(" ", v[i])
  fimpara
  escreval()
fimalgoritmo
