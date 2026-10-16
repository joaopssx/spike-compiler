// Procura o primeiro múltiplo de 7 maior que 50 e para assim que acha.
algoritmo "primeiro multiplo"
var
  n : inteiro
inicio
  n <- 51
  enquanto verdadeiro faca
    se n mod 7 = 0 entao
      interrompa
    fimse
    n <- n + 1
  fimenquanto
  escreval("O primeiro múltiplo de 7 maior que 50 é ", n)
fimalgoritmo
