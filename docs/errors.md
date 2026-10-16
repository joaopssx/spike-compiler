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

**Como corrigir:** use um comando válido: atribuição (`<-`), `escreva`, `escreval`, `leia`, `se`, `enquanto`, `para`, `repita` ou a chamada de um procedimento, `nome(...)`. Um nome seguido de `(` é sempre lido como chamada; se o procedimento não existir, o erro é o E780.

```
algoritmo "exemplo"
inicio
  imprima "oi"
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

## E306 — leia com algo que não é variável

**Mensagem:** `nome de variável esperado em leia`

**Quando acontece:** dentro do `leia` há um número, um texto ou uma conta. O `leia` guarda o que foi digitado, então só aceita nomes de variáveis.

**Como corrigir:** escreva só nomes de variáveis, separados por vírgula: `leia(x, y)`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  leia(1)
fimalgoritmo
```

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

## E710 — operador aritmético com tipos errados

**Mensagem:** `o operador '+' não funciona entre 'caractere' e 'inteiro'`

**Quando acontece:** `+`, `-`, `*`, `/` ou `^` foi usado com um valor que não é número (a única exceção é `+` entre dois textos, que junta os dois), ou `div` e `mod` foram usados com um valor que não é `inteiro`.

**Como corrigir:** use números dos dois lados; em `div` e `mod`, números inteiros. Para dividir reais, use `/`. Para montar uma mensagem com número, passe os valores separados para o `escreva` (`escreval("idade: ", idade)`) ou converta o número com `paraTexto()`.

```
algoritmo "exemplo"
var
  nome : caractere
inicio
  nome <- "idade: " + 18
fimalgoritmo
```

## E711 — comparação com tipos incompatíveis

**Mensagem:** `não é possível comparar 'caractere' com 'inteiro' usando '>'`

**Quando acontece:** uma comparação juntou tipos diferentes, ou usou `<`, `>`, `<=` ou `>=` com valores do tipo `logico`. Números se comparam com números (inteiro com real vale), textos com textos, e `logico` só com `=` e `<>`.

**Como corrigir:** compare valores do mesmo tipo.

```
algoritmo "exemplo"
var
  nome : caractere
  ok : logico
inicio
  ok <- nome > 5
fimalgoritmo
```

## E712 — 'e' ou 'ou' com valor que não é logico

**Mensagem:** `o operador 'e' precisa de dois valores do tipo logico, mas recebeu 'inteiro'`

**Quando acontece:** um dos lados de `e` ou `ou` não é `verdadeiro`/`falso`.

**Como corrigir:** transforme o valor numa comparação: `x > 0 e verdadeiro`.

```
algoritmo "exemplo"
var
  x : inteiro
  ok : logico
inicio
  ok <- x e verdadeiro
fimalgoritmo
```

## E713 — 'nao' ou '-' com o tipo errado

**Mensagem:** `o operador 'nao' precisa de um valor do tipo logico, mas recebeu 'inteiro'` ou `o operador '-' precisa de um número, mas recebeu 'caractere'`

**Quando acontece:** `nao` foi usado com algo que não é `logico`, ou o sinal de menos foi colocado na frente de algo que não é número.

**Como corrigir:** use `nao` com condições (`nao (x = 0)`) e o sinal de menos só com `inteiro` e `real`.

```
algoritmo "exemplo"
var
  x : inteiro
  ok : logico
inicio
  ok <- nao x
fimalgoritmo
```

## E720 — atribuição com tipo incompatível

**Mensagem:** `não é possível guardar 'caractere' em 'x', que é 'inteiro'`

**Quando acontece:** o valor do lado direito do `<-` tem um tipo diferente do da variável. A seta aponta para o começo do valor. Guardar `inteiro` numa variável `real` é permitido.

**Como corrigir:** guarde um valor do mesmo tipo da variável, ou declare a variável com o tipo certo.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- "dez"
fimalgoritmo
```

## E721 — real guardado em inteiro

**Mensagem:** `'x' é inteiro e o valor é real`

**Quando acontece:** um valor `real` foi guardado numa variável `inteiro`. O Spike não corta as casas decimais sozinho, porque isso esconderia erros. Lembre que `/` sempre dá `real`: `7 / 2` é `3.5`.

**Como corrigir:** declare a variável como `real`, ou converta o valor com `trunca()` (corta as casas decimais) ou `arredonda()` (arredonda para o inteiro mais próximo).

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- 7 / 2
fimalgoritmo
```

## E730 — condição que não é logica

**Mensagem:** `a condição do 'se' precisa ser logica, mas é 'inteiro'` (o comando citado pode ser `se`, `enquanto` ou `repita`)

**Quando acontece:** a condição de um `se`, `enquanto` ou do `ate` de um `repita` não dá `verdadeiro` ou `falso`. No Spike, um número não vale como condição.

**Como corrigir:** escreva uma comparação. Se a condição era uma variável inteira, a dica sugere `x <> 0` ou `x = 1`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  se x entao
    escreval("x")
  fimse
fimalgoritmo
```

## E731 — variável do para não é inteiro

**Mensagem:** `a variável do 'para' precisa ser 'inteiro', mas 'r' é 'real'`

**Quando acontece:** a variável que conta as voltas do `para` foi declarada com outro tipo.

**Como corrigir:** declare a variável do `para` como `inteiro`.

```
algoritmo "exemplo"
var
  r : real
inicio
  para r de 1 ate 3 faca
    escreval(r)
  fimpara
fimalgoritmo
```

## E732 — início, fim ou passo do para não é inteiro

**Mensagem:** `o fim do 'para' precisa ser 'inteiro', mas é 'real'` (ou `o início`, `o passo`)

**Quando acontece:** o valor inicial, o valor final ou o passo do `para` não é um `inteiro`. Lembre que `/` sempre dá `real`.

**Como corrigir:** use valores inteiros. Para um valor real, converta com `trunca()` ou `arredonda()`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i de 1 ate 2.5 faca
    escreval(i)
  fimpara
fimalgoritmo
```

## E733 — passo 0

**Mensagem:** `'passo 0' faz o 'para' repetir para sempre`

**Quando acontece:** o `para` foi escrito com `passo 0`. A variável nunca muda e o laço nunca termina. Se o passo for uma variável que vale zero, o Spike só percebe quando o programa roda.

**Como corrigir:** use um passo diferente de zero: `passo 1` para contar para cima, `passo -1` para contar para baixo.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i de 1 ate 10 passo 0 faca
    escreval(i)
  fimpara
fimalgoritmo
```

---

# Erros de execução

Estes erros acontecem quando o programa já compilou e está rodando. A mensagem começa com `erro de execução[R...]`, mostra a linha do `.por` onde aconteceu, e o programa termina com o código de saída 101.

## R001 — divisão por zero

**Mensagem:** `divisão por zero`

**Quando acontece:** o divisor de `/`, `div` ou `mod` vale zero no momento da conta.

**Como corrigir:** confira o divisor antes de dividir: `se b <> 0 entao ... fimse`.

## R002 — índice fora do vetor

**Mensagem:** `índice 11 fora do vetor 'notas' (válido: 1 a 5)` (numa matriz: `índice 5 fora da dimensão 2 de 'm' (válido: 1 a 4)`)

**Quando acontece:** com o programa rodando, um índice calculado (por exemplo `notas[i]` com `i` valendo 11) fica fora dos limites da declaração. Numa matriz, cada índice é conferido com a sua própria dimensão, e a mensagem diz qual delas falhou.

**Como corrigir:** confira os limites do `para` ou da conta que calcula o índice. Um erro comum é o laço ir até um a mais do que o tamanho do vetor.

## R003 — entrada inválida

**Mensagem:** `esperava um número inteiro, mas recebeu 'abc'` (ou `esperava um número`, ou `esperava verdadeiro ou falso (ou v, f, sim, nao)`; para uma linha em branco, `mas recebeu uma linha vazia`)

**Quando acontece:** o `leia` recebeu um texto que não combina com o tipo da variável. Por exemplo, `abc` para uma variável `inteiro`, ou `3.5` para um `inteiro`.

**Como corrigir:** digite um valor do tipo certo, um por linha. Para `real`, valem `3.5` e `3,5`.

## R005 — resultado inválido numa conta

**Mensagem:** `resultado inválido na potência`

**Quando acontece:** uma potência não tem resultado que caiba num número real: `0 ^ -1` daria infinito, e `(-8) ^ 0.5` (raiz quadrada de negativo) não é um número real.

**Como corrigir:** confira a base e o expoente antes da conta. Base zero não pode ter expoente negativo, e base negativa só aceita expoente inteiro.

## R006 — passo zero no para

**Mensagem:** `o passo do 'para' não pode ser zero`

**Quando acontece:** o `passo` de um `para` é uma variável ou conta que vale zero quando o laço começa. Com passo zero a variável nunca muda e o laço nunca terminaria. Quando o `passo 0` está escrito direto no programa, o Spike já avisa antes de compilar (E733).

**Como corrigir:** garanta que o passo seja diferente de zero antes do `para`, por exemplo com `se p <> 0 entao ... fimse`.

## R007 — a entrada terminou

**Mensagem:** `a entrada acabou antes de leia() conseguir ler 'b'`

**Quando acontece:** o programa pediu um valor, mas não havia mais nada para ler. Isso aparece quando a entrada vem de um arquivo (`programa < dados.txt`) com menos linhas do que o programa precisa.

**Como corrigir:** coloque uma linha na entrada para cada valor que o programa lê.

## R008 — memória insuficiente

**Mensagem:** `memória insuficiente`

**Quando acontece:** o programa criou textos demais e o computador ficou sem memória. Na prática, só acontece num laço que junta textos sem parar.

**Como corrigir:** confira se o laço termina.

## E740 — alterar uma constante

**Mensagem:** `não é possível alterar a constante 'PI'`

**Quando acontece:** o programa tenta mudar o valor de uma constante: com `<-`, com `leia`, ou usando a constante como variável de um `para`.

**Como corrigir:** se o valor precisa mudar, declare o nome na seção `var` em vez de `const`.

```
algoritmo "exemplo"
const
  PI = 3.14
inicio
  PI <- 3
fimalgoritmo
```

## E741 — valor de constante que não é fixo

**Mensagem:** `o valor de uma constante precisa ser fixo (literais e outras constantes)`

**Quando acontece:** o valor de uma constante usa uma variável, ou uma constante declarada mais abaixo. O valor de uma constante é calculado antes de o programa rodar, então só pode depender de coisas que já são conhecidas.

**Como corrigir:** use só números, textos, `verdadeiro`/`falso` e constantes declaradas nas linhas de cima. Se o valor depende de uma variável, use uma variável em vez de uma constante.

```
algoritmo "exemplo"
const
  N = x + 1
var
  x : inteiro
inicio
fimalgoritmo
```

## E742 — constante que não pode ser calculada

**Mensagem:** `não foi possível calcular a constante 'N': divisão por zero` (ou `resultado inválido na potência`)

**Quando acontece:** o valor da constante tem uma conta impossível, como dividir por zero.

**Como corrigir:** corrija a conta. Como o Spike calcula o valor antes de o programa rodar, o erro aparece já na compilação.

```
algoritmo "exemplo"
const
  N = 10 div 0
inicio
fimalgoritmo
```

## E750 — interrompa fora de um laço

**Mensagem:** `'interrompa' só pode ser usado dentro de um laço`

**Quando acontece:** o `interrompa` aparece fora de qualquer `enquanto`, `repita` ou `para`. Um `se` sozinho não conta como laço.

**Como corrigir:** coloque o `interrompa` dentro do laço que você quer encerrar, ou apague-o.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  se x = 0 entao
    interrompa
  fimse
fimalgoritmo
```

## E760 — valor do escolha com tipo errado

**Mensagem:** `'escolha' aceita inteiro ou caractere, mas o valor é 'real'`

**Quando acontece:** o valor depois de `escolha` é `real` ou `logico`. O `escolha` compara valores exatos, e isso só faz sentido com inteiros e textos.

**Como corrigir:** para um `real`, use `se` com comparações, ou converta com `trunca()`; para um `logico`, use `se ... senao`.

```
algoritmo "exemplo"
var
  r : real
inicio
  escolha r
    caso 1
  fimescolha
fimalgoritmo
```

## E761 — caso com tipo diferente do escolha

**Mensagem:** `o caso tem tipo 'caractere', mas o 'escolha' é de 'inteiro'`

**Quando acontece:** um valor de `caso` não tem o mesmo tipo do valor escolhido.

**Como corrigir:** use valores do mesmo tipo: números para um `escolha` de inteiro, textos para um de caractere.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    caso "um"
  fimescolha
fimalgoritmo
```

## E762 — valor repetido em dois casos

**Mensagem:** `o valor 2 aparece em dois casos`

**Quando acontece:** o mesmo valor aparece em mais de um `caso`, inclusive quando duas faixas se sobrepõem (a mensagem mostra a parte repetida: `os valores de 4 a 5 aparecem em dois casos`).

**Como corrigir:** deixe cada valor em um único `caso`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    caso 1, 2
    caso 2
  fimescolha
fimalgoritmo
```

## E763 — caso com valor que não é fixo

**Mensagem:** `o valor de um caso precisa ser fixo (literal ou constante)`

**Quando acontece:** um `caso` usa uma variável. Os valores dos casos precisam ser conhecidos antes de o programa rodar.

**Como corrigir:** use um número, um texto ou uma constante. Para comparar com uma variável, use `se`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    caso x
  fimescolha
fimalgoritmo
```

## E764 — faixa de trás para frente

**Mensagem:** `o intervalo começa em 9 e termina em 1`

**Quando acontece:** numa faixa `caso a ate b`, o primeiro número é maior que o segundo, então nenhum valor cabe nela.

**Como corrigir:** escreva o menor valor primeiro: `caso 1 ate 9`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    caso 9 ate 1
  fimescolha
fimalgoritmo
```

## E765 — faixa com texto

**Mensagem:** `intervalo com 'ate' só funciona com inteiros`

**Quando acontece:** um `caso` de um `escolha` de texto usa `ate`.

**Como corrigir:** liste cada texto separado por vírgula: `caso "a", "b"`.

```
algoritmo "exemplo"
var
  t : caractere
inicio
  escolha t
    caso "a" ate "c"
  fimescolha
fimalgoritmo
```

## E770 — vetor sem índice

**Mensagem:** `o vetor 'v' precisa de um índice`

**Quando acontece:** um vetor aparece sem `[ ]` numa conta, num `escreva`, numa atribuição ou num `leia`. O Spike não trabalha com o vetor inteiro de uma vez.

**Como corrigir:** use um elemento por vez, como `v[i]`, normalmente dentro de um `para`.

```
algoritmo "exemplo"
var
  v : vetor[1..5] de inteiro
  i : inteiro
  x : real
inicio
  escreval(v)
fimalgoritmo
```

## E771 — índice em algo que não é vetor

**Mensagem:** `'i' não é um vetor`

**Quando acontece:** um nome declarado como variável simples aparece com `[ ]`.

**Como corrigir:** tire o `[ ]`, ou declare o nome como vetor: `i : vetor[1..10] de inteiro`.

```
algoritmo "exemplo"
var
  v : vetor[1..5] de inteiro
  i : inteiro
  x : real
inicio
  i[1] <- 2
fimalgoritmo
```

## E772 — índice que não é inteiro

**Mensagem:** `o índice precisa ser inteiro, mas é 'real'`

**Quando acontece:** o valor dentro de `[ ]` é real, texto ou lógico. As posições de um vetor são números inteiros.

**Como corrigir:** use um inteiro; para um real, `trunca()` o transforma.

```
algoritmo "exemplo"
var
  v : vetor[1..5] de inteiro
  i : inteiro
  x : real
inicio
  escreval(v[x])
fimalgoritmo
```

## E773 — quantidade errada de índices

**Mensagem:** `'v' tem 1 dimensão, mas recebeu 2 índices` (ou `'m' tem 2 dimensões, mas recebeu 1 índice`)

**Quando acontece:** o número de valores dentro de `[ ]` não bate com a declaração do vetor. Uma matriz `vetor[1..3, 1..4]` precisa de dois índices: `m[i, j]`.

**Como corrigir:** use um índice para cada dimensão da declaração.

```
algoritmo "exemplo"
var
  v : vetor[1..5] de inteiro
  i : inteiro
  x : real
inicio
  escreval(v[1, 2])
fimalgoritmo
```

## E774 — índice fixo fora do vetor

**Mensagem:** `índice 6 fora do vetor 'notas' (válido: 1 a 5)` (numa matriz: `índice 5 fora da dimensão 2 de 'm' (válido: 1 a 4)`)

**Quando acontece:** o índice é um número (ou conta só com constantes) que fica fora dos limites da declaração. Como o valor já é conhecido, o Spike avisa antes de compilar. Numa matriz, cada índice é comparado com a sua dimensão.

**Como corrigir:** use um índice entre os limites, ou aumente o vetor.

```
algoritmo "exemplo"
var
  notas : vetor[1..5] de real
inicio
  notas[6] <- 7
fimalgoritmo
```

## E775 — limites do vetor inválidos

**Mensagem:** `o vetor 'w' termina antes de começar (de 3 a 1)`

**Quando acontece:** o fim do vetor é menor que o início, o vetor teria mais de 10 milhões de elementos (numa matriz, contando todas as dimensões juntas; dentro de um procedimento, o limite é 10 mil, e a mensagem diz `o vetor local 'v' é grande demais`), ou um limite usa algo que não é uma constante inteira (a mensagem diz qual dos três).

**Como corrigir:** escreva o menor limite primeiro, use um vetor menor (ou declare o vetor grande no `var` do programa, fora do procedimento), ou use números inteiros e constantes inteiras nos limites.

```
algoritmo "exemplo"
var
  w : vetor[3..1] de inteiro
inicio
fimalgoritmo
```

## E780 — procedimento não definido

**Mensagem:** `procedimento 'saudar' não foi definido`

**Quando acontece:** a linha chama `nome(...)`, mas não existe procedimento com esse nome. Também acontece quando o nome é de uma variável: só procedimentos são chamados com `( )`.

**Como corrigir:** confira o nome (letra trocada é o mais comum) ou declare o procedimento antes do `inicio` do programa.

```
algoritmo "exemplo"
inicio
  saudar("Ana")
fimalgoritmo
```

## E781 — quantidade errada de argumentos

**Mensagem:** `'saudar' espera 2 argumentos, recebeu 1`

**Quando acontece:** a chamada passa mais ou menos valores do que o procedimento tem de parâmetros.

**Como corrigir:** passe um valor para cada parâmetro, na mesma ordem da declaração. A dica mostra a linha onde o procedimento foi declarado.

```
algoritmo "exemplo"
procedimento saudar(nome: caractere, vezes: inteiro)
inicio
fimprocedimento
inicio
  saudar("Ana")
fimalgoritmo
```

## E782 — argumento de tipo errado

**Mensagem:** `o argumento 1 de 'saudar' precisa ser 'caractere', mas é 'inteiro'`

**Quando acontece:** o valor passado não combina com o tipo do parâmetro. Um `inteiro` pode ir para um parâmetro `real`; o contrário não.

**Como corrigir:** passe um valor do tipo do parâmetro. Para mandar um real para um parâmetro inteiro, use `trunca()` ou `arredonda()`.

```
algoritmo "exemplo"
procedimento saudar(nome: caractere, vezes: inteiro)
inicio
fimprocedimento
inicio
  saudar(3, 2)
fimalgoritmo
```

## E783 — procedimento declarado duas vezes

**Mensagem:** `procedimento 'p' já foi declarado` (ou `'total' já foi declarada como variável`, quando o nome já é de uma variável ou constante global)

**Quando acontece:** dois procedimentos têm o mesmo nome (maiúsculas não contam), ou o procedimento tem o nome de uma variável ou constante do programa.

**Como corrigir:** apague uma das declarações ou dê outro nome ao procedimento.

```
algoritmo "exemplo"
procedimento p()
inicio
fimprocedimento
procedimento p()
inicio
fimprocedimento
inicio
fimalgoritmo
```

## E784 — procedimento usado como valor

**Mensagem:** `'mostrar' é um procedimento, não uma variável`

**Quando acontece:** o nome de um procedimento aparece numa conta, depois de `<-`, à esquerda de `<-` ou dentro de `leia`. Um procedimento não guarda nem devolve valor.

**Como corrigir:** para chamar o procedimento, escreva `mostrar()` sozinho na linha. Se precisava de um valor, use uma variável.

```
algoritmo "exemplo"
var
  x : inteiro
procedimento mostrar()
inicio
fimprocedimento
inicio
  x <- mostrar
fimalgoritmo
```

## E800 — falta o nome da constante

**Mensagem:** `nome da constante esperado` (ou `nome da constante esperado depois de 'const'`, quando a seção está vazia)

**Quando acontece:** uma linha da seção `const` não começa com um nome, ou a palavra `const` aparece sem nenhuma constante depois.

**Como corrigir:** escreva uma constante por linha, no formato `NOME = valor`. Se não precisa de constantes, apague a linha `const`.

```
algoritmo "exemplo"
const
  = 3
inicio
fimalgoritmo
```

## E801 — falta o '=' na constante

**Mensagem:** `'=' esperado depois do nome da constante 'PI'`

**Quando acontece:** depois do nome da constante não veio `=`. Constantes usam `=`, não `<-` nem `:`.

**Como corrigir:** escreva `PI = 3.14`.

```
algoritmo "exemplo"
const
  PI 3.14
inicio
fimalgoritmo
```

## E802 — falta o valor da constante

**Mensagem:** `valor esperado para a constante 'PI'`

**Quando acontece:** a linha termina logo depois do `=`.

**Como corrigir:** coloque o valor na mesma linha: `PI = 3.14`.

```
algoritmo "exemplo"
const
  PI =
inicio
fimalgoritmo
```

## E810 — falta o caso depois do escolha

**Mensagem:** `'caso' esperado depois de 'escolha'`

**Quando acontece:** depois da linha do `escolha` veio um comando, e não um `caso`, `outrocaso` ou `fimescolha`.

**Como corrigir:** comece cada bloco com `caso valor`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    x <- 1
  fimescolha
fimalgoritmo
```

## E811 — falta o fimescolha

**Mensagem:** `'fimescolha' não encontrado`

**Quando acontece:** o `escolha` não foi fechado.

**Como corrigir:** termine o bloco com `fimescolha`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    caso 1
fimalgoritmo
```

## E812 — outrocaso fora do lugar

**Mensagem:** `'outrocaso' repetido`

**Quando acontece:** há dois `outrocaso` no mesmo `escolha`, ou um `caso` depois do `outrocaso` (a mensagem é então `'caso' depois de 'outrocaso'`).

**Como corrigir:** use no máximo um `outrocaso`, sempre como o último bloco.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    outrocaso
    caso 1
  fimescolha
fimalgoritmo
```

## E813 — caso sem valor

**Mensagem:** `valor esperado depois de 'caso'`

**Quando acontece:** a palavra `caso` está sozinha na linha.

**Como corrigir:** escreva um valor (`caso 1`), uma lista (`caso 2, 3`) ou uma faixa (`caso 4 ate 6`) na mesma linha.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
    caso
  fimescolha
fimalgoritmo
```

## E814 — escolha sem casos

**Mensagem:** `'escolha' sem nenhum caso`

**Quando acontece:** o `escolha` não tem nenhum `caso` nem `outrocaso`.

**Como corrigir:** acrescente pelo menos um `caso`, ou apague o `escolha`.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  escolha x
  fimescolha
fimalgoritmo
```

## E820 — falta o '[' do vetor

**Mensagem:** `'[' esperado depois de 'vetor'`

**Quando acontece:** depois de `vetor` não veio `[`.

**Como corrigir:** escreva `vetor[1..10] de tipo`.

```
algoritmo "exemplo"
var
  v : vetor 1..5] de real
inicio
fimalgoritmo
```

## E821 — limite do vetor faltando

**Mensagem:** `limite do vetor esperado (um número inteiro ou uma constante)`

**Quando acontece:** no lugar de um limite veio outra coisa, como um número real ou nada.

**Como corrigir:** use um número inteiro (pode ser negativo) ou uma constante inteira.

```
algoritmo "exemplo"
var
  v : vetor[..5] de real
inicio
fimalgoritmo
```

## E822 — falta o '..' entre os limites

**Mensagem:** `'..' esperado entre os limites do vetor`

**Quando acontece:** os dois limites não estão separados por `..`.

**Como corrigir:** escreva os dois pontos colados: `vetor[1..5]`.

```
algoritmo "exemplo"
var
  v : vetor[1 5] de real
inicio
fimalgoritmo
```

## E823 — falta o ']' na declaração

**Mensagem:** `']' esperado depois dos limites do vetor`

**Quando acontece:** os limites do vetor não foram fechados com `]`.

**Como corrigir:** feche com `]`: `vetor[1..5] de real`.

```
algoritmo "exemplo"
var
  v : vetor[1..5 de real
inicio
fimalgoritmo
```

## E824 — falta o 'de' do vetor

**Mensagem:** `'de' esperado depois de ']'`

**Quando acontece:** depois de `]` veio o tipo sem a palavra `de`.

**Como corrigir:** escreva `vetor[1..5] de real`.

```
algoritmo "exemplo"
var
  v : vetor[1..5] real
inicio
fimalgoritmo
```

## E825 — falta o ']' no uso do vetor

**Mensagem:** `']' esperado depois do índice de 'v'`

**Quando acontece:** o índice de um vetor foi aberto com `[` e não foi fechado.

**Como corrigir:** feche com `]`: `v[1]`.

```
algoritmo "exemplo"
var
  v : vetor[1..5] de real
inicio
  v[1 <- 2
fimalgoritmo
```

## E826 — dimensões demais

**Mensagem:** `um vetor pode ter no máximo 3 dimensões`

**Quando acontece:** a declaração tem quatro ou mais pares de limites.

**Como corrigir:** use até três dimensões: `vetor[1..2, 1..3, 1..4]`.

```
algoritmo "exemplo"
var
  m : vetor[1..2, 1..2, 1..2, 1..2] de inteiro
inicio
fimalgoritmo
```

## E827 — falta a vírgula entre as dimensões

**Mensagem:** `',' esperado entre as dimensões do vetor`

**Quando acontece:** dois pares de limites foram escritos um depois do outro, sem vírgula.

**Como corrigir:** separe as dimensões com vírgula: `vetor[1..3, 1..4]`.

```
algoritmo "exemplo"
var
  m : vetor[1..3 1..4] de inteiro
inicio
fimalgoritmo
```

## E830 — falta o nome do procedimento

**Mensagem:** `nome do procedimento esperado depois de 'procedimento'`

**Quando acontece:** a palavra `procedimento` não é seguida de um nome.

**Como corrigir:** escreva o nome logo depois: `procedimento saudar(nome: caractere)`.

```
algoritmo "exemplo"
procedimento (a: inteiro)
inicio
fimprocedimento
inicio
fimalgoritmo
```

## E831 — parâmetros mal formados

**Mensagem:** `'(' esperado depois do nome do procedimento 'p'` (ou `':' esperado depois do parâmetro 'a'`, `tipo do parâmetro 'a' esperado`, `')' esperado no fim dos parâmetros de 'p'`, `nome do parâmetro esperado`, `um parâmetro ainda não pode ser um vetor`)

**Quando acontece:** o cabeçalho do procedimento não segue o formato `procedimento nome(a: tipo, b: tipo)`. A mensagem diz o que faltou.

**Como corrigir:** abra e feche os parênteses (mesmo sem parâmetros: `procedimento linha()`), escreva `nome: tipo` para cada parâmetro e separe-os com vírgula. Um vetor não pode ser parâmetro por enquanto: declare-o no `var` do programa, que o procedimento enxerga.

```
algoritmo "exemplo"
procedimento p(a: )
inicio
fimprocedimento
inicio
fimalgoritmo
```

## E832 — falta o 'fimprocedimento'

**Mensagem:** `'fimprocedimento' esperado para fechar 'p'`

**Quando acontece:** o corpo do procedimento chegou ao `inicio` do programa, a outro `procedimento` ou ao fim do arquivo sem `fimprocedimento`.

**Como corrigir:** feche cada procedimento com `fimprocedimento` antes do próximo procedimento ou do `inicio` do programa.

```
algoritmo "exemplo"
procedimento p()
inicio
  escreval(1)
inicio
  p()
fimalgoritmo
```

## E833 — procedimento depois do 'inicio'

**Mensagem:** `procedimento declarado depois do 'inicio' do programa`

**Quando acontece:** um procedimento foi escrito dentro do corpo do programa.

**Como corrigir:** mova o procedimento para antes do `inicio` do programa, logo depois da seção `var`.

```
algoritmo "exemplo"
inicio
  escreval(1)
procedimento p()
inicio
fimprocedimento
fimalgoritmo
```

## E834 — falta o ')' na chamada

**Mensagem:** `')' esperado no fim da chamada de 'p'`

**Quando acontece:** a chamada de um procedimento abriu `(` e não fechou.

**Como corrigir:** feche a chamada: `p(1, 2)`.

```
algoritmo "exemplo"
procedimento p(a: inteiro)
inicio
fimprocedimento
inicio
  p(1
fimalgoritmo
```

---

# Erros do gerador e do compilador C

Estes erros não são do seu programa: ou o Spike tem um defeito, ou falta alguma coisa no computador.

## E900 — erro interno do gerador

**Mensagem:** `erro interno do gerador: expressão sem tipo válido` (ou outra descrição começando com `erro interno do gerador`)

**Quando acontece:** o gerador de C encontrou algo que a análise deveria ter barrado. Nunca deveria acontecer com um programa que passou na análise.

**Como corrigir:** é um defeito do Spike. Abra uma issue em https://github.com/joaopssx/spike-compiler/issues com o arquivo `.por` que provoca o erro.

## E901 — comando ainda não suportado pelo gerador

**Mensagem:** `comando ainda não suportado pelo gerador: 'leia'`

**Quando acontece:** o programa usa um comando que o Spike já entende, mas ainda não sabe transformar em C. Enquanto a v0.2 não fica pronta, isso vale para `leia`, `se`, `enquanto`, `para` e `repita`.

**Como corrigir:** por enquanto, não há como. O comando passa a funcionar numa versão próxima.

## E902 — nenhum compilador C encontrado

**Mensagem:** `nenhum compilador C encontrado (procurei em SPIKE_CC e no PATH)` ou `SPIKE_CC aponta para '...', mas esse programa não foi encontrado`

**Quando acontece:** o Spike transforma o programa em C e precisa de um compilador C (GCC ou Clang) para gerar o executável. Ele procura primeiro na variável de ambiente `SPIKE_CC` e depois no `PATH`.

**Como corrigir:**
- No Linux: `sudo apt install build-essential` (ou instale o `clang`).
- No macOS: `xcode-select --install`.
- No Windows: instale o MinGW-w64 (por exemplo pelo MSYS2) e coloque a pasta `bin` dele no `PATH`, ou defina `SPIKE_CC` com o caminho do `gcc.exe`.
- Se você definiu `SPIKE_CC`, confira o caminho ou apague a variável.

## E903 — o compilador C recusou o código gerado

**Mensagem:** `o compilador C recusou o código gerado`

**Quando acontece:** o C que o Spike gerou não compilou. Se o programa passou na análise do Spike, isso é sempre um defeito do Spike, nunca do seu programa. A mensagem mostra as 20 primeiras linhas do que o compilador C reclamou.

**Como corrigir:** rode de novo com `--c` para guardar o arquivo C e abra uma issue em https://github.com/joaopssx/spike-compiler/issues com o `.por` e o `.c`.

## E904 — não foi possível gravar um arquivo

**Mensagem:** `não foi possível gravar '...'`

**Quando acontece:** o Spike não conseguiu criar o arquivo C temporário, a pasta temporária ou o executável. Em geral é falta de permissão na pasta, disco cheio, ou o executável ainda aberto de uma execução anterior (no Windows, um programa rodando não pode ser sobrescrito).

**Como corrigir:** feche o programa se ele ainda estiver rodando, confira se você pode escrever na pasta e se há espaço em disco.