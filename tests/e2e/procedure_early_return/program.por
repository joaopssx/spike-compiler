algoritmo "saída antecipada"
procedimento mostra_positivo(n: inteiro)
inicio
  se n <= 0 entao
    escreval("ignorado")
    retorne
  fimse
  escreval("valor: ", n)
fimprocedimento
inicio
  mostra_positivo(5)
  mostra_positivo(-1)
  mostra_positivo(2)
fimalgoritmo
