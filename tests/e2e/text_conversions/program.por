algoritmo "conversões"
var
  entrada : caractere
  total : real
  i : inteiro
inicio
  para i de 1 ate 3 faca
    leia(entrada)
    se ehNumero(entrada) entao
      total <- total + paraReal(entrada)
    senao
      escreval("ignorado: ", entrada)
    fimse
  fimpara
  escreval("total: ", total)
  escreval("texto: " + paraTexto(total) + " / " + paraTexto(total > 10) + " / " + paraTexto(trunca(total)))
  escreval(ord("A"), " ", chr(ord("A") + 1), " ", chr(231))
fimalgoritmo
