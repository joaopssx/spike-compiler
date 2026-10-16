algoritmo "escolha"
const
  SETE = 7
var
  i : inteiro
  nome : caractere
inicio
  para i de 1 ate 8 faca
    escolha i
      caso 1
        escreva("um ")
      caso 2, 3
        escreva("dois-tres ")
      caso 4 ate 6
        escreva("faixa ")
      caso SETE
        interrompa
      outrocaso
        escreva("outro ")
    fimescolha
  fimpara
  escreval("| i = ", i)
  nome <- "ana"
  escolha nome + "!"
    caso "ana!", "bia!"
      escreval("conhecida")
    outrocaso
      escreval("desconhecida")
  fimescolha
  escolha 99
    caso 1
      escreval("nunca")
  fimescolha
  escolha i mod 2
    caso 0
      escreval("par")
    outrocaso
      escreval("ímpar")
  fimescolha
fimalgoritmo
