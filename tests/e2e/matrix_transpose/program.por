algoritmo "transposta"
var
  a, t : vetor[1..3, 1..3] de inteiro
  i, j : inteiro
inicio
  para i de 1 ate 3 faca
    para j de 1 ate 3 faca
      a[i, j] <- (i - 1) * 3 + j
    fimpara
  fimpara
  para i de 1 ate 3 faca
    para j de 1 ate 3 faca
      t[j, i] <- a[i, j]
    fimpara
  fimpara
  para i de 1 ate 3 faca
    escreval(t[i, 1], " ", t[i, 2], " ", t[i, 3])
  fimpara
fimalgoritmo
