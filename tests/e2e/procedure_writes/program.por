algoritmo "saudação"
procedimento saudar(nome: caractere, vezes: inteiro)
var
  i : inteiro
inicio
  para i de 1 ate vezes faca
    escreval("Oi, ", nome, "!")
  fimpara
fimprocedimento
inicio
  saudar("Ana", 2)
  saudar("Bruno", 1)
  saudar("Caio", 0)
fimalgoritmo
