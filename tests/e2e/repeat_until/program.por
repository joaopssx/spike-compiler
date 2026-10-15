// Pede uma senha até o usuário acertar, usando repita.
algoritmo "senha"
var
  senha : inteiro
inicio
  repita
    escreva("Digite a senha: ")
    leia(senha)
  ate senha = 1234
  escreval("Acesso liberado.")
fimalgoritmo
