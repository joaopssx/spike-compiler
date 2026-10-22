algoritmo "dados"
var
  i, valor : inteiro
  fora : logico
inicio
  escreva(aleatorio(1, 6))
  para i de 2 ate 10 faca
    escreva(" ", aleatorio(1, 6))
  fimpara
  escreval()
  para i de 1 ate 1000 faca
    valor <- aleatorio(-3, 3)
    se (valor < -3) ou (valor > 3) entao
      fora <- verdadeiro
    fimse
  fimpara
  escreval("fora do intervalo: ", fora)
fimalgoritmo
