algoritmo "busca"
var
  nomes : vetor[0..3] de caractere
  temperaturas : vetor[-2..2] de inteiro
  i, achou : inteiro
  procurado : caractere
inicio
  nomes[0] <- "ana"
  nomes[1] <- "bia"
  nomes[3] <- "caio"
  escreval("[", nomes[2], "]")
  procurado <- "caio"
  achou <- -1
  para i de 0 ate 3 faca
    se nomes[i] = procurado entao
      achou <- i
      interrompa
    fimse
  fimpara
  escreval(procurado, " está na posição ", achou)
  para i de -2 ate 2 faca
    temperaturas[i] <- i * 10
  fimpara
  escreval(temperaturas[-2], " ", temperaturas[0], " ", temperaturas[2])
fimalgoritmo
