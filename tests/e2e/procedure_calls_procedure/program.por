algoritmo "moldura"
procedimento titulo(texto: caractere)
inicio
  linha(10)
  escreval(texto)
  linha(10)
fimprocedimento
procedimento linha(tamanho: inteiro)
var
  i : inteiro
  traco : caractere
inicio
  para i de 1 ate tamanho faca
    traco <- traco + "-"
  fimpara
  escreval(traco)
fimprocedimento
inicio
  titulo("Relatório")
  linha(3)
fimalgoritmo
