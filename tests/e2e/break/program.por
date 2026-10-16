algoritmo "interrompa"
var
  n, i, j : inteiro
inicio
  n <- 51
  enquanto verdadeiro faca
    se n mod 7 = 0 entao
      interrompa
    fimse
    n <- n + 1
  fimenquanto
  escreval("primeiro múltiplo de 7 maior que 50: ", n)
  para i de 1 ate 10 faca
    se i > 3 entao
      interrompa
    fimse
    escreva(i, " ")
  fimpara
  escreval("| i = ", i)
  para i de 1 ate 3 faca
    para j de 1 ate 3 faca
      se j > i entao
        interrompa
      fimse
      escreva(i, j, " ")
    fimpara
  fimpara
  escreval()
  repita
    n <- n + 1
    se n = 60 entao
      interrompa
    fimse
  ate falso
  escreval("n = ", n)
fimalgoritmo
