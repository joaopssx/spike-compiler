// Lê cinco notas num vetor e mostra a soma e a média.
algoritmo "soma das notas"
const
  TOTAL = 5
var
  notas : vetor[1..TOTAL] de real
  i : inteiro
  soma : real
inicio
  para i de 1 ate TOTAL faca
    escreva("Nota ", i, ": ")
    leia(notas[i])
  fimpara
  para i de 1 ate TOTAL faca
    soma <- soma + notas[i]
  fimpara
  escreval("Soma: ", soma)
  escreval("Média: ", soma / TOTAL)
fimalgoritmo
