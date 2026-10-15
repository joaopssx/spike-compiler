# Catálogo de erros

Cada erro do Spike tem um código. O código aparece no começo da mensagem (`erro[E400]`) e é o jeito mais rápido de achar a explicação aqui.

- **E001 a E004**: o texto do programa tem um caractere ou trecho que o Spike não reconhece.
- **E100 a E600**: as palavras estão certas, mas a estrutura do programa não.
- **E700 em diante**: a estrutura está certa, mas o programa não faz sentido (por exemplo, usa uma variável que não existe).

---

## E001 — caractere desconhecido

**Mensagem:** `caractere desconhecido '@'`

**Quando acontece:** o programa tem um símbolo que o Portugol não usa, como `@`, `#`, `$` ou `;`. Letras com acento fora de um texto entre aspas também dão este erro, porque nomes de variáveis só aceitam letras sem acento.

**Como corrigir:** apague o símbolo ou troque pelo certo. Em nomes, escreva sem acento: `media` em vez de `média`.

```
algoritmo "exemplo"
inicio
  @
fimalgoritmo
```

## E002 — string não fechada

**Mensagem:** `string não fechada`

**Quando acontece:** um texto abriu aspas e a linha acabou antes de fechar. Um texto não pode ocupar duas linhas.

**Como corrigir:** feche as aspas na mesma linha. Para quebrar a linha na tela, use `escreval` ou `\n` dentro do texto.

```
algoritmo "exemplo"
inicio
  escreva("olá
fimalgoritmo
```

## E003 — número malformado

**Mensagem:** `número malformado: '.' sem dígito decimal` ou `número malformado: múltiplos pontos decimais`

**Quando acontece:** um número termina em ponto (`3.`) ou tem mais de um ponto (`1.2.3`).

**Como corrigir:** escreva `3.0` em vez de `3.`, e use um ponto só.

```
algoritmo "exemplo"
var
  x : real
inicio
  x <- 3.
fimalgoritmo
```

## E004 — comentário de bloco não fechado

**Mensagem:** `comentário de bloco não fechado`

**Quando acontece:** um comentário abriu com `{` e o arquivo acabou sem `}`.

**Como corrigir:** feche o comentário com `}`.

```
algoritmo "exemplo"
{ este comentário nunca fecha
inicio
fimalgoritmo
```

## E100 — algoritmo esperado

**Mensagem:** `algoritmo esperado`

**Quando acontece:** o arquivo não começa com `algoritmo`.

**Como corrigir:** a primeira linha do programa deve ser `algoritmo "nome"`.

```
inicio
  escreval("oi")
fimalgoritmo
```

## E101 — nome do algoritmo esperado

**Mensagem:** `nome do algoritmo esperado após 'algoritmo'`

**Quando acontece:** depois de `algoritmo` não veio um nome entre aspas.

**Como corrigir:** coloque o nome entre aspas duplas: `algoritmo "calculadora"`.

```
algoritmo calculadora
inicio
fimalgoritmo
```

## E102 — inicio esperado

**Mensagem:** `inicio esperado`

**Quando acontece:** depois do nome do programa e da seção `var` não veio `inicio`.

**Como corrigir:** abra o corpo do programa com `inicio`.

```
algoritmo "exemplo"
var
  x : inteiro
  x <- 1
fimalgoritmo
```

## E103 — fimalgoritmo esperado

**Mensagem:** `fimalgoritmo esperado`

**Quando acontece:** o programa acabou sem `fimalgoritmo`. Hoje este erro também aparece depois de outro erro, mesmo quando o `fimalgoritmo` está no arquivo. Nesse caso, corrija o primeiro erro e compile de novo.

**Como corrigir:** termine o programa com `fimalgoritmo`.

```
algoritmo "exemplo"
inicio
  escreval("oi")
```

## E104 — tipo inválido

**Mensagem:** `tipo inválido 'texto'`

**Quando acontece:** uma variável foi declarada com um tipo que não existe.

**Como corrigir:** use um dos tipos válidos: `inteiro`, `real`, `caractere` ou `logico`.

```
algoritmo "exemplo"
var
  nome : texto
inicio
fimalgoritmo
```

## E105 — declaração de variável malformada

**Mensagem:** `':' esperado na declaração de variável` ou `esperava nome de variável após ','`

**Quando acontece:** uma linha da seção `var` não segue o formato `nome1, nome2 : tipo`. Faltou o `:` ou sobrou uma vírgula.

**Como corrigir:** escreva os nomes separados por vírgula, depois `:` e o tipo.

```
algoritmo "exemplo"
var
  x inteiro
inicio
fimalgoritmo
```

## E200 — parêntese não fechado

**Mensagem:** `')' esperado`

**Quando acontece:** uma expressão abriu `(` e não fechou.

**Como corrigir:** feche o parêntese antes de continuar.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- (1 + 2
fimalgoritmo
```

## E201 — expressão esperada

**Mensagem:** `expressão esperada`

**Quando acontece:** o Spike esperava um valor (número, texto, variável ou expressão entre parênteses) e achou outra coisa, como um operador sozinho ou o fim da linha. Hoje a seta aponta para o começo da linha seguinte quando a expressão falta no fim da linha.

**Como corrigir:** complete a expressão.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- * 2
fimalgoritmo
```

## E300 — instrução não reconhecida

**Mensagem:** `instrução não reconhecida: 'imprima'`

**Quando acontece:** uma linha do corpo não começa com um comando que o Spike conhece.

**Como corrigir:** use um comando válido: atribuição (`<-`), `escreva`, `escreval`, `leia`, `se`, `enquanto`, `para` ou `repita`.

```
algoritmo "exemplo"
inicio
  imprima("oi")
fimalgoritmo
```

## E301 — falta '(' no escreva

**Mensagem:** `'(' esperado após escreva` (ou `escreval`)

**Quando acontece:** `escreva` ou `escreval` sem parênteses.

**Como corrigir:** coloque os valores entre parênteses: `escreva("texto", variavel)`.

```
algoritmo "exemplo"
inicio
  escreval "oi"
fimalgoritmo
```

## E302 — falta ')' no escreva

**Mensagem:** `')' esperado no final do escreva` (ou `escreval`)

**Quando acontece:** os parênteses do `escreva` abriram e não fecharam.

**Como corrigir:** feche com `)`.

```
algoritmo "exemplo"
inicio
  escreval("oi"
fimalgoritmo
```

## E303 — falta '(' no leia

**Mensagem:** `'(' esperado após leia`

**Quando acontece:** `leia` sem parênteses.

**Como corrigir:** escreva as variáveis entre parênteses: `leia(x, y)`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  leia x
fimalgoritmo
```

## E304 — falta ')' no leia

**Mensagem:** `')' esperado no final do leia`

**Quando acontece:** os parênteses do `leia` abriram e não fecharam.

**Como corrigir:** feche com `)`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  leia(x
fimalgoritmo
```

## E305 — falta o símbolo de atribuição

**Mensagem:** `'<-' ou ':=' esperado na atribuição`

**Quando acontece:** uma atribuição sem `<-` ou `:=` depois do nome da variável. Na prática, o Spike só reconhece uma linha como atribuição quando o símbolo está lá, então este erro é raro: o mais comum é o E300.

**Como corrigir:** use `x <- 42` ou `x := 42`.

## E400 — falta 'entao'

**Mensagem:** `'entao' esperado após condição do 'se'`

**Quando acontece:** a condição do `se` não foi seguida de `entao`.

**Como corrigir:** escreva `se condição entao`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  se x > 1
    x <- 2
  fimse
fimalgoritmo
```

## E401 — falta 'fimse'

**Mensagem:** `'fimse' não encontrado — todo 'se' precisa de um 'fimse'`

**Quando acontece:** um `se` não foi fechado.

**Como corrigir:** feche o bloco com `fimse`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  se x > 1 entao
    x <- 2
fimalgoritmo
```

## E500 — falta 'faca' no enquanto

**Mensagem:** `'faca' esperado após condição do 'enquanto'`

**Quando acontece:** a condição do `enquanto` não foi seguida de `faca`.

**Como corrigir:** escreva `enquanto condição faca`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  enquanto x < 10
    x <- x + 1
  fimenquanto
fimalgoritmo
```

## E501 — falta 'fimenquanto'

**Mensagem:** `'fimenquanto' não encontrado`

**Quando acontece:** um `enquanto` não foi fechado.

**Como corrigir:** feche o laço com `fimenquanto`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  enquanto x < 10 faca
    x <- x + 1
fimalgoritmo
```

## E502 — falta a variável do para

**Mensagem:** `nome de variável esperado após 'para'`

**Quando acontece:** depois de `para` não veio o nome da variável contadora.

**Como corrigir:** escreva `para i de 1 ate 10 faca`.

```
algoritmo "exemplo"
inicio
  para de 1 ate 10 faca
  fimpara
fimalgoritmo
```

## E503 — falta 'de' no para

**Mensagem:** `'de' esperado: sintaxe é 'para i de 1 ate 10 faca'`

**Quando acontece:** depois da variável do `para` não veio `de`.

**Como corrigir:** escreva `para i de 1 ate 10 faca`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i 1 ate 10 faca
  fimpara
fimalgoritmo
```

## E504 — falta 'ate' no para

**Mensagem:** `'ate' esperado após valor inicial do 'para'`

**Quando acontece:** depois do valor inicial do `para` não veio `ate`.

**Como corrigir:** escreva `para i de 1 ate 10 faca`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i de 1 10 faca
  fimpara
fimalgoritmo
```

## E505 — falta 'faca' no para

**Mensagem:** `'faca' esperado após 'ate [valor]'`

**Quando acontece:** depois do valor final (ou do `passo`) não veio `faca`.

**Como corrigir:** termine a linha com `faca`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i de 1 ate 10
  fimpara
fimalgoritmo
```

## E506 — falta 'fimpara'

**Mensagem:** `'fimpara' não encontrado`

**Quando acontece:** um `para` não foi fechado.

**Como corrigir:** feche o laço com `fimpara`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i de 1 ate 10 faca
    escreval(i)
fimalgoritmo
```

## E600 — falta 'ate' no repita

**Mensagem:** `'ate' esperado para fechar 'repita'`

**Quando acontece:** um `repita` não terminou com `ate condição`.

**Como corrigir:** feche o laço com `ate` e a condição de parada. O corpo roda pelo menos uma vez e para quando a condição fica verdadeira.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  repita
    x <- x + 1
fimalgoritmo
```

## E700 — variável não declarada

**Mensagem:** `variável 'y' não foi declarada`

**Quando acontece:** o programa usa um nome que não está na seção `var`. Vale para qualquer lugar: atribuição, expressão, `escreva`, `leia`, condição e `para`. Se o mesmo nome aparece várias vezes na mesma linha, o erro aparece uma vez só.

**Como corrigir:** declare a variável na seção `var`, ou confira se o nome está escrito igual à declaração. Maiúsculas e minúsculas não importam: `Idade` e `idade` são a mesma variável.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  y <- x + 1
fimalgoritmo
```

## E701 — variável declarada duas vezes

**Mensagem:** `variável 'x' já foi declarada`

**Quando acontece:** o mesmo nome aparece duas vezes na seção `var`, mesmo com maiúsculas diferentes (`total` e `TOTAL`). A dica mostra a linha da primeira declaração.

**Como corrigir:** apague uma das declarações ou dê outro nome a uma delas.

```
algoritmo "exemplo"
var
  x : inteiro
  x : real
inicio
fimalgoritmo
```
