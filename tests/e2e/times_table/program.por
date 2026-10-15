// Mostra a tabuada de um número usando para com passo.
algoritmo "tabuada"
var
  numero, i : inteiro
inicio
  escreva("Tabuada de qual número? ")
  leia(numero)
  para i de 1 ate 10 passo 1 faca
    escreval(numero, " x ", i, " = ", numero * i)
  fimpara
fimalgoritmo
