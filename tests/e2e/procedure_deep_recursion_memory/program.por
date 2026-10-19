algoritmo "pilha cheia"
procedimento desce(n: inteiro)
var
  pesado : vetor[1..10000] de real
inicio
  pesado[1] <- n
  desce(n + 1)
fimprocedimento
inicio
  desce(1)
fimalgoritmo
