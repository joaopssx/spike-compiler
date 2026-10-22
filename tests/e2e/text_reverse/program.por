algoritmo "inverter"
var
  texto, invertido : caractere
  i : inteiro
inicio
  texto <- "Ação rápida"
  para i de tamanho(texto) ate 1 passo -1 faca
    invertido <- invertido + copia(texto, i, 1)
  fimpara
  escreval(invertido)
  escreval(tamanho(texto), " ", tamanho(invertido))
fimalgoritmo
