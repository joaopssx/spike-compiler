// Lê uma palavra e diz se ela é um palíndromo (igual de trás para frente).
algoritmo "palíndromo"
var
  palavra : caractere
funcao inverte(t: caractere): caractere
var
  i : inteiro
  resultado : caractere
inicio
  para i de tamanho(t) ate 1 passo -1 faca
    resultado <- resultado + copia(t, i, 1)
  fimpara
  retorne resultado
fimfuncao
inicio
  escreva("Palavra: ")
  leia(palavra)
  palavra <- minusculo(palavra)
  se palavra = inverte(palavra) entao
    escreval(palavra, " é um palíndromo")
  senao
    escreval(palavra, " não é um palíndromo")
  fimse
fimalgoritmo
