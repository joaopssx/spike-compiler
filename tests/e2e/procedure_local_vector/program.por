algoritmo "vetor local"
procedimento quadrados(n: inteiro)
var
  q : vetor[1..5] de inteiro
  i : inteiro
inicio
  para i de 1 ate n faca
    q[i] <- q[i] + i * i
  fimpara
  escreval(q[1], " ", q[n])
fimprocedimento
inicio
  quadrados(3)
  quadrados(5)
fimalgoritmo
