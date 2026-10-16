algoritmo "menu"
var
  opcao : inteiro
  a, b : real
inicio
  escreval("1 - somar")
  escreval("2 - subtrair")
  escreval("3 - multiplicar")
  escreval("4 - dividir")
  escreva("Opção: ")
  leia(opcao)
  a <- 12
  b <- 4
  escolha opcao
    caso 1
      escreval(a, " + ", b, " = ", a + b)
    caso 2
      escreval(a, " - ", b, " = ", a - b)
    caso 3
      escreval(a, " * ", b, " = ", a * b)
    caso 4
      escreval(a, " / ", b, " = ", a / b)
    outrocaso
      escreval("Opção inválida.")
  fimescolha
fimalgoritmo
