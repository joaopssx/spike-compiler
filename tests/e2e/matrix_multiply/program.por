// Multiplica duas matrizes 2x2 e mostra o resultado linha por linha.
algoritmo "multiplicação de matrizes"
const
  N = 2
var
  a, b, c : vetor[1..N, 1..N] de inteiro
  i, j, k : inteiro
inicio
  a[1, 1] <- 1
  a[1, 2] <- 2
  a[2, 1] <- 3
  a[2, 2] <- 4
  b[1, 1] <- 5
  b[1, 2] <- 6
  b[2, 1] <- 7
  b[2, 2] <- 8
  para i de 1 ate N faca
    para j de 1 ate N faca
      c[i, j] <- 0
      para k de 1 ate N faca
        c[i, j] <- c[i, j] + a[i, k] * b[k, j]
      fimpara
    fimpara
  fimpara
  para i de 1 ate N faca
    escreval(c[i, 1], " ", c[i, 2])
  fimpara
fimalgoritmo
