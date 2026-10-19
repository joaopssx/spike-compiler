algoritmo "incremento"
var
  contador : inteiro
  notas : vetor[1..3] de real
procedimento incrementa(var n: inteiro)
inicio
  n <- n + 1
fimprocedimento
procedimento aumenta(var nota: real, bonus: real)
inicio
  nota <- nota + bonus
fimprocedimento
inicio
  incrementa(contador)
  incrementa(contador)
  incrementa(contador)
  escreval("contador: ", contador)
  notas[2] <- 7.5
  aumenta(notas[2], 1)
  aumenta(notas[2], 0.25)
  escreval("nota: ", notas[2])
fimalgoritmo
