algoritmo "maior valor"
var
  valores : vetor[1..6] de inteiro
  i, maior, posicao : inteiro
inicio
  valores[1] <- 12
  valores[2] <- -4
  valores[3] <- 37
  valores[4] <- 8
  valores[5] <- 37
  valores[6] <- 21
  maior <- valores[1]
  posicao <- 1
  para i de 2 ate 6 faca
    se valores[i] > maior entao
      maior <- valores[i]
      posicao <- i
    fimse
  fimpara
  escreval("Maior valor: ", maior, " na posição ", posicao)
fimalgoritmo
