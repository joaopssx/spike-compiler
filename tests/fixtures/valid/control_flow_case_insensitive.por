Algoritmo fluxo
VAR
  ativo: Logico
Inicio
  ativo <- Verdadeiro
  Se NAO ativo Ou ativo E FALSO Entao
    escreva("ramo 1")
  Senao
    escreva("ramo 2")
  FimSe
  Enquanto ativo <> falso Faca
    ativo <- falso
  FimEnquanto
FimAlgoritmo
