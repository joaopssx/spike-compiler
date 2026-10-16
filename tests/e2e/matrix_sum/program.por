algoritmo "soma de matriz"
var
  a : vetor[1..2, 1..3] de inteiro
  i, j, soma, linha : inteiro
inicio
  soma <- 0
  para i de 1 ate 2 faca
    linha <- 0
    para j de 1 ate 3 faca
      a[i, j] <- i * 10 + j
      linha <- linha + a[i, j]
    fimpara
    escreval("linha ", i, ": ", linha)
    soma <- soma + linha
  fimpara
  escreval("total: ", soma)
  escreval(a[1, 3], " ", a[2, 1])
fimalgoritmo
