// Procedimentos: blocos com nome que o programa chama quantas vezes quiser.
algoritmo "procedimentos"
var
  total : inteiro
procedimento linha(tamanho: inteiro)
var
  i : inteiro
  traco : caractere
inicio
  para i de 1 ate tamanho faca
    traco <- traco + "="
  fimpara
  escreval(traco)
fimprocedimento
procedimento item(nome: caractere, preco: inteiro)
inicio
  escreval(nome, ": R$ ", preco)
  total <- total + preco
fimprocedimento
inicio
  linha(20)
  item("Caderno", 15)
  item("Caneta", 3)
  item("Mochila", 80)
  linha(20)
  escreval("Total: R$ ", total)
fimalgoritmo
