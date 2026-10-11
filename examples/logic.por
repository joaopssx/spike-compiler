// Combina e, ou, nao e comparações em condições.
algoritmo "logica"
var
  ativo : logico
  idade : inteiro
inicio
  ativo <- verdadeiro
  idade <- 17
  se nao ativo ou ativo e falso entao
    escreval("falso")
  senao
    escreval("verdadeiro")
  fimse
  se idade >= 16 e idade < 18 entao
    escreval("voto facultativo")
  fimse
  se (idade <= 0) ou (idade > 130) entao
    escreval("idade inválida")
  fimse
fimalgoritmo
