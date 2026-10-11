// Classifica uma nota com se dentro de se.
algoritmo "conceito"
var
  nota : real
inicio
  escreva("Digite a nota: ")
  leia(nota)
  se nota >= 7 entao
    escreval("Aprovado")
  senao
    se nota >= 5 entao
      escreval("Recuperação")
    senao
      escreval("Reprovado")
    fimse
  fimse
fimalgoritmo
