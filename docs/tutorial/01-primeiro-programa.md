# 1. O primeiro programa

Todo programa em Portugol tem a mesma forma: um nome, um começo e um fim.

```portugol-run
algoritmo "ola"
inicio
  escreval("Olá, mundo!")
  escreval("Este é o meu primeiro programa.")
fimalgoritmo
```

```output
Olá, mundo!
Este é o meu primeiro programa.
```

Linha por linha:

- `algoritmo "ola"` dá um nome ao programa. O nome fica entre aspas.
- `inicio` marca onde os comandos começam.
- `escreval(...)` escreve o texto e pula para a linha seguinte.
- `fimalgoritmo` termina o programa.

Salve o programa como `ola.por` e compile:

```
spike ola.por
```

O Spike cria o executável `ola.exe` (no Windows) ou `ola` (no Linux e no macOS), na mesma pasta. Para rodar, use `.\ola.exe` no Windows ou `./ola` nos outros sistemas. Esse arquivo funciona sozinho: dá para copiar para outro computador, sem o Spike instalado.

Para compilar e rodar de uma vez:

```
spike ola.por --run
```

## escreva e escreval

`escreva` escreve e continua na mesma linha. `escreval` escreve e pula a linha. Os dois aceitam vários valores separados por vírgula:

```portugol-run
algoritmo "linhas"
inicio
  escreva("um, ")
  escreva("dois, ")
  escreval("três")
  escreval("A soma de 2 e 3 é ", 2 + 3)
fimalgoritmo
```

```output
um, dois, três
A soma de 2 e 3 é 5
```

Linhas que começam com `//` são comentários: o Spike ignora o resto da linha. Use para deixar anotações.

## Exercício

Escreva um programa que mostre seu nome numa linha e, na linha seguinte, quantos anos você terá daqui a 10 anos, calculando a conta no próprio `escreval`.

## Solução

```portugol-run
algoritmo "sobre mim"
inicio
  // troque pelo seu nome e pela sua idade
  escreval("Meu nome é Ana")
  escreval("Daqui a 10 anos terei ", 19 + 10, " anos")
fimalgoritmo
```

```output
Meu nome é Ana
Daqui a 10 anos terei 29 anos
```
