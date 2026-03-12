algoritmo "nested"

var
    a: inteiro
    b: inteiro

inicio
    a <- 5
    b <- 3

    se a > b entao
        se b < a entao
            escreva(a)
        fimse
    senao
        escreva(b)
    fimse
fimalgoritmo
