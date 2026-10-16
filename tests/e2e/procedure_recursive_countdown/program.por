algoritmo "contagem"
procedimento contagem(n: inteiro)
inicio
  se n = 0 entao
    escreval("fogo!")
  senao
    escreval(n)
    contagem(n - 1)
  fimse
fimprocedimento
inicio
  contagem(5)
fimalgoritmo
