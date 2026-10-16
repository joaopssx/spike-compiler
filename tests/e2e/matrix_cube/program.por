algoritmo "cubo"
var
  c : vetor[1..2, 1..3, 1..2] de inteiro
  i, j, k, soma : inteiro
inicio
  para i de 1 ate 2 faca
    para j de 1 ate 3 faca
      para k de 1 ate 2 faca
        c[i, j, k] <- i * 100 + j * 10 + k
      fimpara
    fimpara
  fimpara
  para i de 1 ate 2 faca
    para j de 1 ate 3 faca
      para k de 1 ate 2 faca
        soma <- soma + c[i, j, k]
      fimpara
    fimpara
  fimpara
  escreval(c[2, 3, 1], " ", c[1, 1, 2], " ", c[2, 1, 2])
  escreval("soma: ", soma)
fimalgoritmo
