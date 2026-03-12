algoritmo "fibonacci"

var
    n: inteiro
    i: inteiro
    atual: inteiro
    proximo: inteiro
    temp: inteiro

inicio
    n <- 8
    i <- 0
    atual <- 0
    proximo <- 1

    enquanto i < n faca
        escreva(atual)
        temp <- atual + proximo
        atual <- proximo
        proximo <- temp
        i <- i + 1
    fimenquanto
fimalgoritmo
