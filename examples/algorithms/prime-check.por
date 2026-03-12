algoritmo "prime_check"

var
    n: inteiro
    i: inteiro
    divisores: inteiro

inicio
    n <- 7
    i <- 1
    divisores <- 0

    enquanto i <= n faca
        se (n / i) = (n / i) entao
            escreva(i)
        fimse
        i <- i + 1
    fimenquanto
fimalgoritmo
