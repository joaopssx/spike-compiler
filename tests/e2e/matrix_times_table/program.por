algoritmo "tabuada em matriz"
var
  tab : vetor[1..9, 1..10] de inteiro
  i, j, linha3, total : inteiro
inicio
  para i de 1 ate 9 faca
    para j de 1 ate 10 faca
      tab[i, j] <- i * j
    fimpara
  fimpara
  escreval("7 x 8 = ", tab[7, 8])
  escreval("9 x 10 = ", tab[9, 10])
  para j de 1 ate 10 faca
    linha3 <- linha3 + tab[3, j]
  fimpara
  escreval("linha 3: ", linha3)
  para i de 1 ate 9 faca
    para j de 1 ate 10 faca
      total <- total + tab[i, j]
    fimpara
  fimpara
  escreval("total: ", total)
fimalgoritmo
