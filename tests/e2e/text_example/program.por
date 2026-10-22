// Funções prontas de texto: tamanho, maiúsculas, cópia, busca e conversões.
algoritmo "textos"
var
  nome, sigla : caractere
  ano : inteiro
inicio
  nome <- "São Paulo"
  escreval("Texto: ", nome)
  escreval("Letras: ", tamanho(nome))
  escreval("Maiúsculo: ", maiusculo(nome))
  escreval("Minúsculo: ", minusculo(nome))
  sigla <- maiusculo(copia(nome, 1, 1) + copia(nome, 5, 1))
  escreval("Sigla: ", sigla)
  escreval("Posição de 'Paulo': ", pos("Paulo", nome))
  ano <- paraInteiro("2026")
  escreval("Ano que vem: ", ano + 1)
  escreval("Junto: ", concatena("ano ", paraTexto(ano)))
  escreval("'12,5' é número? ", ehNumero("12,5"))
  escreval("Código de 'ç': ", ord("ç"), ", caractere 65: ", chr(65))
fimalgoritmo
