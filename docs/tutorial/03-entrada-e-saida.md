# 3. Entrada e saída

`leia` espera o usuário digitar algo e guarda na variável. Cada `leia` lê uma linha inteira: digite um valor e aperte Enter.

```portugol-run
algoritmo "media"
var
  nota1, nota2 : real
inicio
  escreva("Primeira nota: ")
  leia(nota1)
  escreva("Segunda nota: ")
  leia(nota2)
  escreval("Média: ", (nota1 + nota2) / 2)
fimalgoritmo
```

Digitando `8` e `6,5`:

```input
8
6,5
```

```output
Primeira nota: Segunda nota: Média: 7.25
```

Na tela, cada valor digitado aparece depois da pergunta, porque é você que digita. Aqui a entrada veio de um arquivo, então só a saída do programa aparece.

Algumas regras do `leia`:

- Um número `real` pode ser digitado com vírgula ou com ponto: `6,5` e `6.5` valem o mesmo. Na saída, o Spike sempre usa ponto.
- Um `caractere` recebe a linha inteira, com espaços.
- Um `logico` aceita `verdadeiro`, `falso`, `v`, `f`, `sim` ou `nao`.
- `leia(a, b)` lê duas linhas, uma para cada variável.

Se o usuário digitar algo que não combina com o tipo, como `abc` para um `inteiro`, o programa para com o erro R003 e mostra o que foi digitado.

Para não depender do teclado, você pode mandar a entrada de um arquivo:

```
spike media.por
./media < notas.txt
```

No PowerShell, use `Get-Content notas.txt | .\media.exe`.

## Juntando textos

`+` entre dois textos junta os dois. Para juntar texto com número, passe os dois ao `escreval` separados por vírgula, ou converta o número com `paraTexto`:

```portugol-run
algoritmo "saudacao"
var
  nome : caractere
  idade : inteiro
inicio
  leia(nome)
  leia(idade)
  escreval("Olá, " + nome + "!")
  escreval("Você tem " + paraTexto(idade) + " anos.")
fimalgoritmo
```

```input
Maria Clara
20
```

```output
Olá, Maria Clara!
Você tem 20 anos.
```

## Exercício

Leia uma temperatura em graus Celsius e mostre o valor em Fahrenheit (`F = C * 9 / 5 + 32`).

## Solução

```portugol-run
algoritmo "fahrenheit"
var
  c : real
inicio
  escreva("Temperatura em Celsius: ")
  leia(c)
  escreval(c, " °C = ", c * 9 / 5 + 32, " °F")
fimalgoritmo
```

```input
25
```

```output
Temperatura em Celsius: 25 °C = 77 °F
```
