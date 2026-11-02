# 4. Decisões: se e escolha

## se

`se` roda um bloco só quando a condição é verdadeira. `senao` é opcional e roda quando ela é falsa.

```portugol-run
algoritmo "aprovado"
var
  nota : real
inicio
  leia(nota)
  se nota >= 7 entao
    escreval("Aprovado")
  senao
    se nota >= 5 entao
      escreval("Recuperação")
    senao
      escreval("Reprovado")
    fimse
  fimse
fimalgoritmo
```

```input
5,5
```

```output
Recuperação
```

As comparações são `=`, `<>` (diferente), `<`, `>`, `<=` e `>=`. Igualdade usa um `=` só; `==` é erro. Para juntar condições, use `e`, `ou` e `nao`:

```portugol-run
algoritmo "faixa"
var
  idade : inteiro
inicio
  idade <- 15
  se idade >= 12 e idade < 18 entao
    escreval("adolescente")
  fimse
  se nao (idade >= 18) entao
    escreval("menor de idade")
  fimse
fimalgoritmo
```

```output
adolescente
menor de idade
```

## escolha

Quando uma variável pode ter vários valores e cada um pede uma ação, `escolha` fica mais fácil de ler do que vários `se`:

```portugol-run
algoritmo "dia"
var
  dia : inteiro
inicio
  dia <- 7
  escolha dia
    caso 1
      escreval("domingo")
    caso 2 ate 6
      escreval("dia de semana")
    caso 7
      escreval("sábado")
    outrocaso
      escreval("dia inválido")
  fimescolha
fimalgoritmo
```

```output
sábado
```

- Cada `caso` aceita valores separados por vírgula (`caso 2, 3`) ou uma faixa (`caso 2 ate 6`).
- Só o primeiro caso que combina roda. Se nenhum combinar, roda o `outrocaso`, que é opcional.
- O valor do `escolha` pode ser `inteiro` ou `caractere`.

## Exercício

Leia um número inteiro e diga se ele é par ou ímpar. Dica: um número é par quando o resto da divisão por 2 é zero.

## Solução

```portugol-run
algoritmo "par ou impar"
var
  n : inteiro
inicio
  leia(n)
  se n mod 2 = 0 entao
    escreval(n, " é par")
  senao
    escreval(n, " é ímpar")
  fimse
fimalgoritmo
```

```input
17
```

```output
17 é ímpar
```
