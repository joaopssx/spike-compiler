# 2. Variáveis e tipos

Uma variável é um nome para um valor que pode mudar. Toda variável é declarada na seção `var`, antes do `inicio`, com o seu tipo:

| Tipo | Guarda | Exemplo |
| --- | --- | --- |
| `inteiro` | números sem vírgula | `42`, `-7` |
| `real` | números com parte decimal | `3.5`, `0.25` |
| `caractere` | texto | `"Olá"` |
| `logico` | verdadeiro ou falso | `verdadeiro` |

O sinal `<-` guarda um valor na variável (lê-se "recebe"):

```portugol-run
algoritmo "tipos"
var
  idade : inteiro
  altura : real
  nome : caractere
  estudante : logico
inicio
  idade <- 19
  altura <- 1.72
  nome <- "Ana"
  estudante <- verdadeiro
  escreval(nome, " tem ", idade, " anos e ", altura, " m de altura")
  escreval("Estuda? ", estudante)
  idade <- idade + 1
  escreval("Ano que vem: ", idade)
fimalgoritmo
```

```output
Ana tem 19 anos e 1.72 m de altura
Estuda? verdadeiro
Ano que vem: 20
```

No código, a parte decimal usa ponto: `1.72`. Toda variável começa com um valor padrão (`0`, `0.0`, `""` ou `falso`), e o Spike não diferencia maiúsculas de minúsculas: `Idade` e `idade` são a mesma variável. Nomes usam só letras sem acento: `media`, e não `média`.

## Contas

`+`, `-` e `*` funcionam como na matemática. A divisão tem três formas:

- `/` sempre dá um `real`: `7 / 2` é `3.5`.
- `div` é a divisão inteira: `7 div 2` é `3`.
- `mod` é o resto: `7 mod 2` é `1`.

`^` é a potência: `2 ^ 10` é `1024`.

```portugol-run
algoritmo "contas"
var
  a, b : inteiro
inicio
  a <- 7
  b <- 2
  escreval(a, " / ", b, " = ", a / b)
  escreval(a, " div ", b, " = ", a div b)
  escreval(a, " mod ", b, " = ", a mod b)
  escreval("2 ^ 10 = ", 2 ^ 10)
  escreval("(1 + 2) * 3 = ", (1 + 2) * 3)
fimalgoritmo
```

```output
7 / 2 = 3.5
7 div 2 = 3
7 mod 2 = 1
2 ^ 10 = 1024
(1 + 2) * 3 = 9
```

Um `inteiro` pode ir para uma variável `real`, mas o contrário não: para guardar `3.7` num `inteiro`, escolha como arredondar com `trunca(3.7)` (dá 3) ou `arredonda(3.7)` (dá 4).

Valores que nunca mudam podem ir numa seção `const`, antes de `var`:

```portugol
algoritmo "circulo"
const
  PI = 3.14159
var
  raio : real
inicio
  raio <- 2
  escreval("Área: ", PI * raio ^ 2)
fimalgoritmo
```

## Exercício

Guarde um total de 135 minutos numa variável e mostre quantas horas e quantos minutos isso dá (`2 h e 15 min`).

## Solução

```portugol-run
algoritmo "horas"
var
  total : inteiro
inicio
  total <- 135
  escreval(total div 60, " h e ", total mod 60, " min")
fimalgoritmo
```

```output
2 h e 15 min
```
