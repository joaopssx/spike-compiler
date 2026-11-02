# 5. Repetição: enquanto, repita, para e interrompa

## enquanto

Repete enquanto a condição for verdadeira. A condição é testada antes de cada volta, então o bloco pode nem rodar.

```portugol-run
algoritmo "contagem"
var
  n : inteiro
inicio
  n <- 3
  enquanto n > 0 faca
    escreval(n, "...")
    n <- n - 1
  fimenquanto
  escreval("Já!")
fimalgoritmo
```

```output
3...
2...
1...
Já!
```

Se nada dentro do laço mudar a condição, ele nunca termina. Num programa travado assim, aperte Ctrl+C no terminal.

## repita

Roda o bloco pelo menos uma vez e para quando a condição do `ate` fica verdadeira. Serve bem para pedir um valor até ele ser válido:

```portugol-run
algoritmo "senha"
var
  senha : inteiro
inicio
  repita
    escreval("Digite a senha:")
    leia(senha)
  ate senha = 1234
  escreval("Entrou!")
fimalgoritmo
```

```input
1111
1234
```

```output
Digite a senha:
Digite a senha:
Entrou!
```

## para

Quando você sabe quantas voltas quer, use `para`. A variável conta sozinha, de um valor até outro:

```portugol-run
algoritmo "tabuada"
var
  i : inteiro
inicio
  para i de 1 ate 5 faca
    escreval("7 x ", i, " = ", 7 * i)
  fimpara
  para i de 10 ate 0 passo -5 faca
    escreval(i)
  fimpara
fimalgoritmo
```

```output
7 x 1 = 7
7 x 2 = 14
7 x 3 = 21
7 x 4 = 28
7 x 5 = 35
10
5
0
```

Sem `passo`, a variável aumenta de 1 em 1. Com `passo -5`, ela diminui.

## interrompa

`interrompa` sai do laço na hora. O programa continua depois do `fimenquanto`, `fimpara` ou `ate`:

```portugol-run
algoritmo "soma ate zero"
var
  n, soma : inteiro
inicio
  enquanto verdadeiro faca
    leia(n)
    se n = 0 entao
      interrompa
    fimse
    soma <- soma + n
  fimenquanto
  escreval("Soma: ", soma)
fimalgoritmo
```

```input
4
10
-2
0
```

```output
Soma: 12
```

## Exercício

Mostre a soma dos números de 1 a 100.

## Solução

```portugol-run
algoritmo "soma de 1 a 100"
var
  i, soma : inteiro
inicio
  para i de 1 ate 100 faca
    soma <- soma + i
  fimpara
  escreval(soma)
fimalgoritmo
```

```output
5050
```
