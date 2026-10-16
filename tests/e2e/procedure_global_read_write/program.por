algoritmo "global"
var
  soma, quantidade : inteiro
  media : real
procedimento ler_valores(n: inteiro)
var
  i, valor : inteiro
inicio
  para i de 1 ate n faca
    leia(valor)
    soma <- soma + valor
  fimpara
  quantidade <- quantidade + n
fimprocedimento
procedimento mostrar()
inicio
  media <- soma / quantidade
  escreval("soma: ", soma, ", média: ", media)
fimprocedimento
inicio
  ler_valores(3)
  mostrar()
  ler_valores(1)
  mostrar()
fimalgoritmo
