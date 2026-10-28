algoritmo "ordem de avaliação"
var
  resto, contador, i : inteiro
  v : vetor[1..3] de inteiro
funcao divide(a, b: inteiro, var r: inteiro): inteiro
inicio
  r <- a mod b
  retorne a div b
fimfuncao
funcao proximo(): inteiro
inicio
  contador <- contador + 1
  retorne contador
fimfuncao
funcao avanca(var n: inteiro): inteiro
inicio
  n <- n + 1
  retorne n
fimfuncao
funcao le(): inteiro
var
  n : inteiro
inicio
  leia(n)
  retorne n
fimfuncao
inicio
  resto <- 100
  escreval(divide(17, 5, resto) + resto)
  resto <- 100
  escreval(resto + divide(17, 5, resto))
  escreval(proximo() - proximo())
  i <- 1
  v[i] <- avanca(i)
  escreval(v[1], " ", v[2], " ", i)
  contador <- 0
  enquanto contador * 2 < proximo() faca
    escreval("enquanto ", contador)
  fimenquanto
  escreval("contador ", contador)
  contador <- 0
  repita
    escreval("repita ", contador)
  ate contador * 2 >= proximo()
  se contador > 100 e proximo() + contador > 0 entao
    escreval("não deveria aparecer")
  fimse
  escreval("contador ", contador)
  se contador = 2 e proximo() + contador = 6 entao
    escreval("sim")
  fimse
  escreval(le() - le())
fimalgoritmo
