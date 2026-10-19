algoritmo "maior"
var
  a, b, c : inteiro
funcao maximo(x, y: inteiro): inteiro
inicio
  se x > y entao
    retorne x
  senao
    retorne y
  fimse
fimfuncao
funcao maior_real(x: real, y: real): real
inicio
  se x > y entao
    retorne x
  fimse
  retorne y
fimfuncao
inicio
  a <- 7
  b <- 12
  c <- -3
  escreval(maximo(a, b))
  escreval(maximo(maximo(a, c), maximo(c, -10)))
  se maximo(a, c) = a entao
    escreval("a é maior que c")
  fimse
  escreval(maior_real(a, 2.5) + 1)
fimalgoritmo
