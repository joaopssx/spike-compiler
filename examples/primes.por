// Lista os números primos até 30 com uma função que devolve verdadeiro ou falso.
algoritmo "primos"
var
  n, total : inteiro
funcao primo(n: inteiro): logico
var
  d : inteiro
inicio
  se n < 2 entao
    retorne falso
  fimse
  d <- 2
  enquanto d * d <= n faca
    se n mod d = 0 entao
      retorne falso
    fimse
    d <- d + 1
  fimenquanto
  retorne verdadeiro
fimfuncao
inicio
  escreva("Primos até 30:")
  para n de 1 ate 30 faca
    se primo(n) entao
      escreva(" ", n)
      total <- total + 1
    fimse
  fimpara
  escreval()
  escreval("Total: ", total)
fimalgoritmo
