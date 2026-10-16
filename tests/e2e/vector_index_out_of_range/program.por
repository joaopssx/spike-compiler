algoritmo "fora do vetor"
var
  v : vetor[1..5] de inteiro
  i : inteiro
inicio
  para i de 1 ate 5 faca
    v[i] <- i
  fimpara
  escreval(v[5])
  i <- 11
  escreval(v[i])
fimalgoritmo
