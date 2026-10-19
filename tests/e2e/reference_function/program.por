algoritmo "divisão com resto"
var
  resto, q : inteiro
funcao divide(a, b: inteiro, var r: inteiro): inteiro
inicio
  r <- a mod b
  retorne a div b
fimfuncao
inicio
  q <- divide(17, 5, resto)
  escreval(q, " resto ", resto)
  escreval(divide(9, 3, resto))
  escreval("resto ", resto)
fimalgoritmo
