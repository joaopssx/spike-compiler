algoritmo "vogais"
var
  frase, letra : caractere
  i, vogais : inteiro
inicio
  frase <- minusculo("Programação É Divertida")
  para i de 1 ate tamanho(frase) faca
    letra <- copia(frase, i, 1)
    se pos(letra, "aeiouáéíóúâêôãõ") > 0 entao
      vogais <- vogais + 1
    fimse
  fimpara
  escreval("vogais: ", vogais)
fimalgoritmo
