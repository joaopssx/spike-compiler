# 6. Vetores e matrizes

Um vetor guarda vários valores do mesmo tipo, numerados. Na declaração, você diz de que número a que número vão as posições:

```portugol-run
algoritmo "notas"
const
  TOTAL = 4
var
  notas : vetor[1..TOTAL] de real
  i : inteiro
  soma : real
inicio
  notas[1] <- 7
  notas[2] <- 8.5
  notas[3] <- 6
  notas[4] <- 10
  para i de 1 ate TOTAL faca
    soma <- soma + notas[i]
  fimpara
  escreval("Média: ", soma / TOTAL)
fimalgoritmo
```

```output
Média: 7.875
```

- `notas[i]` é um elemento do vetor. Funciona como uma variável comum: em contas, à esquerda do `<-` e no `leia`.
- O índice precisa ser `inteiro`.
- Usar uma posição fora do vetor, como `notas[5]`, para o programa com o erro R002. Se o índice for fixo no código, o Spike já avisa ao compilar (E774).
- Usar uma constante para o tamanho, como `TOTAL`, facilita mudar o programa depois.

## Matrizes

Uma matriz é um vetor com duas (ou três) dimensões. Cada uso precisa de um índice por dimensão, separados por vírgula:

```portugol-run
algoritmo "tabela"
var
  m : vetor[1..3, 1..3] de inteiro
  i, j : inteiro
inicio
  para i de 1 ate 3 faca
    para j de 1 ate 3 faca
      m[i, j] <- i * j
    fimpara
  fimpara
  para i de 1 ate 3 faca
    escreva(m[i, 1])
    para j de 2 ate 3 faca
      escreva(" | ", m[i, j])
    fimpara
    escreval()
  fimpara
fimalgoritmo
```

```output
1 | 2 | 3
2 | 4 | 6
3 | 6 | 9
```

`escreval()` sem nada só pula a linha.

## Exercício

Leia 5 números inteiros para um vetor e mostre o maior deles.

## Solução

```portugol-run
algoritmo "maior"
var
  v : vetor[1..5] de inteiro
  i, maior : inteiro
inicio
  para i de 1 ate 5 faca
    leia(v[i])
  fimpara
  maior <- v[1]
  para i de 2 ate 5 faca
    se v[i] > maior entao
      maior <- v[i]
    fimse
  fimpara
  escreval("Maior: ", maior)
fimalgoritmo
```

```input
3
-1
42
7
0
```

```output
Maior: 42
```
