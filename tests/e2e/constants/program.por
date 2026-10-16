algoritmo "circulo"
const
  PI = 3.14159
  LIMITE = 3
  TITULO = "Área do círculo"
  ULTIMO = LIMITE * LIMITE
var
  raio : inteiro
  area : real
inicio
  escreval(TITULO)
  para raio de 1 ate LIMITE faca
    area <- PI * raio ^ 2
    escreval("raio ", raio, ": ", area)
  fimpara
  escreval("maior quadrado: ", ULTIMO)
fimalgoritmo
