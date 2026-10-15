// Lê três notas e escreve a média.
algoritmo "media"
var
  nota1, nota2, nota3, media : real
inicio
  escreva("Digite as três notas: ")
  leia(nota1, nota2, nota3)
  media <- (nota1 + nota2 + nota3) / 3
  escreval("Média: ", media)
fimalgoritmo
