# Catálogo de erros

Cada erro do Spike tem um código. O código aparece no começo da mensagem (`erro[E400]`) e é o jeito mais rápido de achar a explicação aqui.

- **E001 a E005**: o texto do programa tem um caractere ou trecho que o Spike não reconhece.
- **E100 a E600**: as palavras estão certas, mas a estrutura do programa não.
- **E700 em diante**: a estrutura está certa, mas o programa não faz sentido (por exemplo, usa uma variável que não existe).
- **W001 a W009**: avisos. O programa compila, mas algo nele parece engano (veja a seção [Avisos](#avisos), no fim).

Quando falta alguma coisa no fim de uma linha (o `entao` depois de `se x > 1`, o valor depois de `x <-`), a seta fica logo depois da última palavra dessa linha, e não no começo da linha seguinte.

O Spike não para no primeiro erro. Os erros de digitação (E001 a E004) aparecem todos de uma vez, e só depois de corrigidos o Spike passa a ler a estrutura. Na estrutura, ele também continua depois de cada erro e mostra os outros que achar, sem inventar erros que são só consequência do primeiro. Quando um bloco (`se`, `enquanto`, `para`, `repita`, `escolha`, procedimento ou função) fica sem fechamento, a seta aponta para a palavra que abriu o bloco. Aparecem no máximo 20 erros; o resto vira uma linha `(mais N erros omitidos)`.

No fim, o Spike resume o que achou (`falhou: 2 erros e 1 aviso`) e sugere o comando que mostra a explicação do primeiro erro sem sair do terminal: `spike --explain E400`. Sem código, `spike --explain` lista todos.

## Formato deste arquivo

Este arquivo é a fonte do `spike --explain`: na compilação do Spike, `cmake/embed_errors.cmake` lê cada entrada e guarda uma cópia dentro do executável. Por isso toda entrada segue o mesmo formato:

- um título `## E400 — descrição curta` (código, travessão, descrição), um por código;
- as partes `**Mensagem:**`, `**Quando acontece:**` e `**Como corrigir:**`, nesta ordem;
- um bloco de código com um exemplo que provoca o erro (o primeiro bloco da entrada é o que o `--explain` mostra).

Títulos sem código (`# Erros de execução`, `## Formato deste arquivo`) separam grupos e não são entradas. O teste `error_docs_consistency` confere que todo código entre aspas em `src/` e `runtime/` tem uma entrada aqui e que toda entrada é usada; um código novo entra neste arquivo no mesmo commit.

---

## E001 — caractere desconhecido

**Mensagem:** `caractere desconhecido '@'`

**Quando acontece:** o programa tem um símbolo que o Portugol não usa, como `@`, `#`, `$` ou `;`. Letras com acento fora de um texto entre aspas também dão este erro, porque nomes de variáveis só aceitam letras sem acento.

**Como corrigir:** apague o símbolo ou troque pelo certo. Em nomes, escreva sem acento: `media` em vez de `média`. Para um nome com acento aparece um erro só, na primeira letra acentuada, e a dica já mostra o nome escrito sem acento.

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

## E005 — arquivo fora do UTF-8

**Mensagem:** `o arquivo não está em UTF-8 válido`

**Quando acontece:** o arquivo não é UTF-8 e também não é um texto em Windows-1252 (esse caso o Spike converte sozinho, com o aviso W010). Em geral o arquivo tem caracteres de controle, mistura duas codificações, ou nem é um arquivo de texto. A seta aponta para o primeiro byte inválido, que aparece como `?` na linha mostrada. Nenhum outro erro é mostrado enquanto este não for corrigido.

**Como corrigir:** salve o arquivo como UTF-8. No Bloco de Notas, use "Salvar como" e escolha "UTF-8" em "Codificação"; no VS Code, clique na codificação na barra de baixo e escolha "Salvar com codificação" e depois "UTF-8".

```
algoritmo "exemplo"
inicio
  escreval("Olá")     // com o arquivo salvo em Windows-1252
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

**Quando acontece:** o Spike esperava um valor (número, texto, variável ou expressão entre parênteses) e achou outra coisa, como um operador sozinho ou o fim da linha. Quando a expressão falta no fim da linha (`x <-` sem nada depois), a seta aponta para o fim dessa linha, e a linha seguinte é lida normalmente.

**Como corrigir:** complete a expressão.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- * 2
fimalgoritmo
```

## E202 — blocos ou parênteses aninhados demais

**Mensagem:** `blocos ou parênteses aninhados demais`

**Quando acontece:** o programa abre mais de 256 níveis, somando blocos (`se` dentro de `se`...) e parênteses ou operadores dentro de uma expressão. Nenhum programa escrito à mão chega perto disso; normalmente é um arquivo gerado ou corrompido. O Spike para de ler o arquivo nesse ponto.

**Como corrigir:** separe a expressão em partes com variáveis auxiliares, ou divida o código em procedimentos.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- ((((((((((((((((((((((((((( ... 1 ... )))))))))))))))))))))))))))
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

**Quando acontece:** uma posição de vetor (`v[1]`) sem `<-` ou `:=` depois. Com um nome simples sem o símbolo (`x 42`), o Spike não reconhece a linha como atribuição e mostra o E300.

**Como corrigir:** use `v[1] <- 42` ou `v[1] := 42`.

```
algoritmo "exemplo"
var
  v : vetor[1..3] de inteiro
inicio
  v[1] 42
fimalgoritmo
```

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

Estes erros acontecem quando o programa já compilou e está rodando. O programa escreve o erro na saída de erros (stderr) e termina com o código de saída 101. O formato é parecido com o dos erros de compilação: o código, a mensagem, o arquivo e a linha, a própria linha do `.por` e uma dica.

```
erro de execução[R001]: divisão por zero
 --> soma.por:12
     |
  12 |   r <- a div b
     |
     = dica: verifique se o divisor pode valer 0 antes de dividir (use um 'se')
```

Quando o erro acontece dentro de um procedimento ou função, o Spike mostra, depois da linha do erro, por onde o programa passou para chegar ali. Cada linha diz qual subprograma estava rodando e em que linha ele foi chamado, do mais recente para o mais antigo. Aparecem no máximo 8; o resto vira `... e mais N`.

```
erro de execução[R001]: divisão por zero
 --> media.por:4
     |
   4 |   retorne a div b
     |
  chamado de 'divide' na linha 8
  chamado de 'media' na linha 12
  chamado de 'relatorio' na linha 16
     = dica: verifique se o divisor pode valer 0 antes de dividir (use um 'se')
```

Se a linha não existe no arquivo (por exemplo, um erro que não vem de nenhum comando), aparece só ` --> arquivo.por:linha`. As cores só são usadas quando a saída de erros é um terminal, e nunca quando a variável de ambiente `NO_COLOR` tem algum valor.

## R001 — divisão por zero

**Mensagem:** `divisão por zero`

**Quando acontece:** o divisor de `/`, `div` ou `mod` vale zero no momento da conta.

**Como corrigir:** confira o divisor antes de dividir: `se b <> 0 entao ... fimse`.

**Dica mostrada:** `verifique se o divisor pode valer 0 antes de dividir (use um 'se')`

```
algoritmo "exemplo"
var
  a, b : inteiro
inicio
  a <- 10
  b <- 0
  escreval(a div b)
fimalgoritmo
```

## R002 — índice fora do vetor

**Mensagem:** `índice 11 fora do vetor 'notas' (válido: 1 a 5)` (numa matriz: `índice 5 fora da dimensão 2 de 'm' (válido: 1 a 4)`)

**Quando acontece:** com o programa rodando, um índice calculado (por exemplo `notas[i]` com `i` valendo 11) fica fora dos limites da declaração. Numa matriz, cada índice é conferido com a sua própria dimensão, e a mensagem diz qual delas falhou.

**Como corrigir:** confira os limites do `para` ou da conta que calcula o índice. Um erro comum é o laço ir até um a mais do que o tamanho do vetor.

**Dica mostrada:** `o índice precisa estar entre os limites declarados; confira o laço ou a conta que calcula o índice`

```
algoritmo "exemplo"
var
  notas : vetor[1..5] de real
  i : inteiro
inicio
  para i de 1 ate 6 faca
    notas[i] <- 0
  fimpara
fimalgoritmo
```

## R003 — entrada inválida

**Mensagem:** `esperava um número inteiro, mas recebeu 'abc'` (ou `esperava um número`, ou `esperava verdadeiro ou falso (ou v, f, sim, nao)`; para uma linha em branco, `mas recebeu uma linha vazia`)

**Quando acontece:** o `leia` recebeu um texto que não combina com o tipo da variável. Por exemplo, `abc` para uma variável `inteiro`, ou `3.5` para um `inteiro`.

**Como corrigir:** digite um valor do tipo certo, um por linha. Para `real`, valem `3.5` e `3,5`.

**Dica mostrada:** `digite um valor do tipo que o 'leia' espera; em números reais vale ponto ou vírgula`

```
algoritmo "exemplo"
var
  idade : inteiro
inicio
  leia(idade)
fimalgoritmo
```

Rodando e digitando `abc`, o programa para com R003.

## R004 — conversão impossível

**Mensagem:** `número grande demais para virar inteiro` (ou `o valor não é um número e não pode virar inteiro`, ou `não foi possível converter 'abc' para inteiro`)

**Quando acontece:** `arredonda`, `trunca`, `piso` ou `teto` recebeu um número real que não cabe num `inteiro` (o limite é perto de 9,2 × 10^18), ou um valor que nem é um número, como o resultado de uma conta inválida. Também acontece quando `paraInteiro` ou `paraReal` recebe um texto que não é um número.

**Como corrigir:** confira a conta que gera o número; se ele precisa mesmo ser tão grande, deixe-o como `real`. Antes de converter um texto, teste com `ehNumero(texto)`.

**Dica mostrada:** `confira se o número cabe em um inteiro ou se o texto tem só um número (use ehNumero antes de converter)`

```
algoritmo "exemplo"
var
  x : real
inicio
  x <- pot(10, 20)
  escreval(arredonda(x))
fimalgoritmo
```

## R005 — resultado inválido numa conta

**Mensagem:** `resultado inválido na potência` (ou `raiz de número negativo`, ou `logaritmo de número que não é positivo`)

**Quando acontece:** uma potência não tem resultado que caiba num número real: `0 ^ -1` daria infinito, e `(-8) ^ 0.5` (raiz quadrada de negativo) não é um número real. Vale também para `pot`, para `raiz` de um número negativo e para `log` de zero ou de um número negativo.

**Como corrigir:** confira a base e o expoente antes da conta. Base zero não pode ter expoente negativo, e base negativa só aceita expoente inteiro. Antes de `raiz` ou `log`, teste o valor com um `se`.

**Dica mostrada:** `confira o valor antes da conta: raiz só de número sem sinal de menos, log só de número positivo, potência sem resultado infinito`

```
algoritmo "exemplo"
var
  x : real
inicio
  x <- -4
  escreval(raiz(x))
fimalgoritmo
```

## R006 — passo zero no para

**Mensagem:** `o passo do 'para' não pode ser zero`

**Quando acontece:** o `passo` de um `para` é uma variável ou conta que vale zero quando o laço começa. Com passo zero a variável nunca muda e o laço nunca terminaria. Quando o `passo 0` está escrito direto no programa, o Spike já avisa antes de compilar (E733).

**Como corrigir:** garanta que o passo seja diferente de zero antes do `para`, por exemplo com `se p <> 0 entao ... fimse`.

**Dica mostrada:** `use um passo positivo para contar para cima ou negativo para contar para baixo`

```
algoritmo "exemplo"
var
  i, p : inteiro
inicio
  p <- 0
  para i de 1 ate 10 passo p faca
    escreval(i)
  fimpara
fimalgoritmo
```

## R007 — a entrada terminou

**Mensagem:** `a entrada acabou antes de leia() conseguir ler 'b'`

**Quando acontece:** o programa pediu um valor, mas não havia mais nada para ler. Isso aparece quando a entrada vem de um arquivo (`programa < dados.txt`) com menos linhas do que o programa precisa.

**Como corrigir:** coloque uma linha na entrada para cada valor que o programa lê.

**Dica mostrada:** `o programa pediu mais valores do que a entrada tinha; confira quantas vezes o 'leia' é executado`

```
algoritmo "exemplo"
var
  a, b : inteiro
inicio
  leia(a)
  leia(b)
fimalgoritmo
```

Rodando com um arquivo de entrada que só tem uma linha, o segundo `leia` dá R007.

## R008 — memória insuficiente

**Mensagem:** `memória insuficiente` (ou `recursão profunda demais (limite de 10000 chamadas), verifique a condição de parada`, ou `recursão profunda demais (a memória para chamadas acabou), verifique a condição de parada`)

**Quando acontece:** o programa criou textos demais e o computador ficou sem memória, o que na prática só acontece num laço que junta textos sem parar. Também acontece quando uma função ou procedimento chama a si mesmo sem parar: mais de 10000 chamadas ao mesmo tempo, ou chamadas com vetores locais grandes que enchem a memória reservada para elas.

**Como corrigir:** confira se o laço termina. Numa recursão, confira o caso de parada (por exemplo `se n <= 1 entao retorne 1`) e se cada chamada se aproxima dele.

**Dica mostrada:** `toda recursão precisa de uma condição de parada que seja alcançada; confira o 'se' que encerra as chamadas`

```
algoritmo "exemplo"
funcao conta(n: inteiro): inteiro
inicio
  retorne conta(n + 1)
fimfuncao
inicio
  escreval(conta(1))
fimalgoritmo
```

## R009 — função terminou sem retornar valor

**Mensagem:** `a função 'sinal' terminou sem retornar um valor`

**Quando acontece:** a função chegou ao `fimfuncao` sem passar por nenhum `retorne`. Costuma ser um `se` que só retorna num dos caminhos.

**Como corrigir:** garanta um `retorne` em todos os caminhos, por exemplo com um `senao`, ou com um `retorne` antes do `fimfuncao`.

**Dica mostrada:** `faça todos os caminhos da função chegarem a um 'retorne' com valor`

```
algoritmo "exemplo"
funcao sinal(n: inteiro): inteiro
inicio
  se n > 0 entao
    retorne 1
  fimse
fimfuncao
inicio
  escreval(sinal(-5))
fimalgoritmo
```

## R010 — sorteio com limites invertidos

**Mensagem:** `aleatorio(10, 1): o primeiro número precisa ser menor ou igual ao segundo`

**Quando acontece:** `aleatorio(a, b)` foi chamado com `a` maior que `b`.

**Como corrigir:** passe o menor número primeiro: `aleatorio(1, 10)`.

**Dica mostrada:** `chame aleatorio(menor, maior), com o primeiro número menor ou igual ao segundo`

```
algoritmo "exemplo"
inicio
  escreval(aleatorio(10, 1))
fimalgoritmo
```

## R011 — posição ou código fora do alcance

**Mensagem:** `copia: a posição inicial precisa ser 1 ou mais, mas é 0` (ou `ord: o texto está vazio`, ou `chr: 0 não é o código de um caractere (válido: 1 a 1114111)`)

**Quando acontece:** uma função de texto recebeu um valor que não faz sentido: `copia` começando antes da primeira letra, `ord` de um texto vazio, ou `chr` com um código que não existe.

**Como corrigir:** em `copia`, as posições começam em 1. Antes de `ord`, confira se o texto não está vazio (`tamanho(t) > 0`). Em `chr`, use um código entre 1 e 1114111.

**Dica mostrada:** `posições em texto vão de 1 até tamanho(texto); em chr use um código de 1 a 1114111`

```
algoritmo "exemplo"
inicio
  escreval(copia("Spike", 0, 2))
fimalgoritmo
```

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

**Mensagem:** `procedimento 'saudar' não foi definido` (dentro de uma expressão: `função 'dobro' não foi definida`)

**Quando acontece:** o programa chama `nome(...)`, mas não existe procedimento nem função com esse nome. Também acontece quando o nome é de uma variável: só procedimentos e funções são chamados com `( )`.

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

**Como corrigir:** passe um valor para cada parâmetro, na mesma ordem da declaração. A dica mostra a linha onde o procedimento foi declarado; numa função pronta, mostra a assinatura, como `raiz(real): real`.

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

**Quando acontece:** o valor passado não combina com o tipo do parâmetro. Um `inteiro` pode ir para um parâmetro `real`; o contrário não. Vale também para as funções prontas: `raiz("16")` dá este erro, e a dica mostra a assinatura.

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

**Mensagem:** `procedimento 'p' já foi declarado` (ou `função 'f' já foi declarada`, ou `'total' já foi declarada como variável`, quando o nome já é de uma variável ou constante global)

**Quando acontece:** dois procedimentos ou funções têm o mesmo nome (maiúsculas não contam; procedimentos e funções dividem os nomes), ou o nome já é de uma variável ou constante do programa.

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

**Mensagem:** `'mostrar' é um procedimento, não uma variável` (ou `'mostrar' é um procedimento e não devolve valor`, ou `'dobro' é uma função, não uma variável`)

**Quando acontece:** o nome de um procedimento aparece numa conta, depois de `<-`, à esquerda de `<-` ou dentro de `leia`; ou um procedimento é chamado dentro de uma expressão, como `x <- mostrar()`. Um procedimento não guarda nem devolve valor. O nome de uma função sem os parênteses também dá este erro.

**Como corrigir:** para chamar o procedimento, escreva `mostrar()` sozinho na linha. Se precisa de um valor, transforme o procedimento em função (`funcao ... : tipo` com `retorne`). Para usar o valor de uma função, chame com parênteses: `dobro(3)`.

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

## E785 — expressão onde precisava de variável

**Mensagem:** `o argumento de 'n' precisa ser uma variável, porque o parâmetro é por referência`

**Quando acontece:** o parâmetro foi declarado com `var`, e a chamada passou um número, uma conta ou o resultado de uma função. Um parâmetro por referência muda a variável de quem chamou, então precisa receber uma variável (ou um elemento de vetor).

**Como corrigir:** guarde o valor numa variável e passe a variável. Se o procedimento não precisa mudar o valor, tire o `var` do parâmetro.

```
algoritmo "exemplo"
var
  x : inteiro
procedimento incrementa(var n: inteiro)
inicio
  n <- n + 1
fimprocedimento
inicio
  incrementa(x + 1)
fimalgoritmo
```

## E786 — constante num parâmetro por referência

**Mensagem:** `'MAX' é uma constante e não pode ir para o parâmetro 'a', que é por referência`

**Quando acontece:** a chamada passou uma constante para um parâmetro com `var`. O procedimento poderia mudar o valor, e constantes nunca mudam.

**Como corrigir:** copie o valor da constante para uma variável e passe a variável, ou tire o `var` do parâmetro.

```
algoritmo "exemplo"
const
  MAX = 3
procedimento p(var a: inteiro)
inicio
fimprocedimento
inicio
  p(MAX)
fimalgoritmo
```

## E787 — tipo diferente num parâmetro por referência

**Mensagem:** `o parâmetro 'n' é por referência e exige 'inteiro', mas recebeu 'real'`

**Quando acontece:** a variável passada não tem exatamente o tipo do parâmetro com `var`. Num parâmetro por valor, um `inteiro` serve para um `real`; por referência, não, porque o procedimento escreveria um tipo na variável de outro.

**Como corrigir:** passe uma variável do mesmo tipo do parâmetro.

```
algoritmo "exemplo"
var
  r : real
procedimento incrementa(var n: inteiro)
inicio
  n <- n + 1
fimprocedimento
inicio
  incrementa(r)
fimalgoritmo
```

## E788 — vetor como parâmetro

**Mensagem:** `vetores ainda não podem ser passados a procedimentos ou funções`

**Quando acontece:** um parâmetro foi declarado como `vetor[...]`, ou a chamada passou um vetor inteiro, sem índice. Isso ainda não existe no Spike.

**Como corrigir:** declare o vetor no `var` do programa: procedimentos e funções enxergam as variáveis globais. Para passar um só elemento, use o índice: `mostra(v[2])`.

```
algoritmo "exemplo"
procedimento soma(v: vetor[1..3] de inteiro)
inicio
fimprocedimento
inicio
fimalgoritmo
```

## E789 — nome de função pronta

**Mensagem:** `'raiz' é uma função pronta do Spike, escolha outro nome`

**Quando acontece:** um procedimento ou função do programa tem o nome de uma função pronta, como `raiz`, `abs` ou `aleatorio` (maiúsculas não contam).

**Como corrigir:** dê outro nome ao seu procedimento ou função, por exemplo `minha_raiz`. Se queria só usar a função pronta, apague a declaração e chame `raiz(...)` direto.

```
algoritmo "exemplo"
funcao raiz(x: real): real
inicio
  retorne x
fimfuncao
inicio
fimalgoritmo
```

## E790 — valor do 'retorne' com tipo errado

**Mensagem:** `a função 'nome' devolve 'caractere', mas o valor é 'inteiro'`

**Quando acontece:** o valor depois de `retorne` não é do tipo declarado no cabeçalho da função. Um `inteiro` numa função `real` é aceito; o contrário não.

**Como corrigir:** devolva um valor do tipo da função, ou mude o tipo no cabeçalho. Para devolver um real numa função inteira, use `trunca()` ou `arredonda()`.

```
algoritmo "exemplo"
funcao nome(): caractere
inicio
  retorne 42
fimfuncao
inicio
fimalgoritmo
```

## E791 — 'retorne' fora de função ou procedimento

**Mensagem:** `'retorne' só pode ser usado dentro de uma função ou procedimento`

**Quando acontece:** há um `retorne` no corpo principal do programa.

**Como corrigir:** apague o `retorne`. O programa principal termina sozinho ao chegar em `fimalgoritmo`; para parar antes, use um `se` em volta do que não deve rodar.

```
algoritmo "exemplo"
inicio
  retorne
fimalgoritmo
```

## E793 — 'retorne' com ou sem valor no lugar errado

**Mensagem:** `a função 'f' precisa devolver um valor` (ou `o procedimento 'p' não devolve valor`)

**Quando acontece:** numa função, o `retorne` está sozinho, sem valor. Num procedimento, o `retorne` tem um valor, mas procedimentos não devolvem nada.

**Como corrigir:** numa função, escreva o valor na mesma linha: `retorne resultado`. Num procedimento, use `retorne` sozinho, ou transforme o procedimento em função se precisa do valor.

```
algoritmo "exemplo"
funcao f(): inteiro
inicio
  retorne
fimfuncao
inicio
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

**Mensagem:** `'(' esperado depois do nome do procedimento 'p'` (ou `':' esperado depois do parâmetro 'a'`, `tipo do parâmetro 'a' esperado`, `')' esperado no fim dos parâmetros do procedimento 'p'`, `nome do parâmetro esperado`)

**Quando acontece:** o cabeçalho do procedimento não segue o formato `procedimento nome(a: tipo, b: tipo)`. A mensagem diz o que faltou.

**Como corrigir:** abra e feche os parênteses (mesmo sem parâmetros: `procedimento linha()`), escreva `nome: tipo` para cada parâmetro e separe-os com vírgula. Para passar por referência, escreva `var` antes do nome: `procedimento troca(var a, b: inteiro)`.

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

## E835 — falta o tipo de retorno da função

**Mensagem:** `':' e o tipo de retorno esperados depois dos parâmetros da função 'f'` (ou `tipo de retorno da função 'f' esperado`, `uma função ainda não pode devolver um vetor`)

**Quando acontece:** o cabeçalho da função não diz, depois dos parênteses, que tipo de valor ela devolve.

**Como corrigir:** escreva `:` e o tipo depois dos parâmetros: `funcao dobro(n: inteiro): inteiro`. Se não precisa devolver valor, use `procedimento`.

```
algoritmo "exemplo"
funcao f(a: inteiro)
inicio
  retorne a
fimfuncao
inicio
fimalgoritmo
```

## E836 — falta o 'fimfuncao'

**Mensagem:** `'fimfuncao' esperado para fechar 'f'`

**Quando acontece:** o corpo da função chegou ao `inicio` do programa, a outro procedimento ou função, ou ao fim do arquivo sem `fimfuncao`. Também aparece quando a função é fechada com `fimprocedimento`.

**Como corrigir:** feche cada função com `fimfuncao`.

```
algoritmo "exemplo"
funcao f(): inteiro
inicio
  retorne 1
inicio
fimalgoritmo
```

---

# Erros do gerador e do compilador C

Estes erros não são do seu programa: ou o Spike tem um defeito, ou falta alguma coisa no computador.

## E900 — erro interno do gerador

**Mensagem:** `erro interno do gerador: expressão sem tipo válido` (ou outra descrição começando com `erro interno do gerador`)

**Quando acontece:** o gerador de C encontrou algo que a análise deveria ter barrado. Nunca deveria acontecer com um programa que passou na análise.

**Como corrigir:** é um defeito do Spike. Abra uma issue em https://github.com/joaopssx/spike-compiler/issues com o arquivo `.por` que provoca o erro.

```
erro[E900]: erro interno do gerador: expressão sem tipo válido
 --> programa.por:5:8
     = dica: isso é um defeito do Spike, não do seu programa.
```

## E901 — comando ainda não suportado pelo gerador

**Mensagem:** `comando ainda não suportado pelo gerador: '...'`

**Quando acontece:** o programa usa um comando que o Spike já entende, mas ainda não sabe transformar em C. Hoje todos os comandos da linguagem viram C; o código fica reservado para comandos novos que entrarem na análise antes do gerador.

**Como corrigir:** por enquanto, não há como. O comando passa a funcionar numa versão próxima.

```
erro[E901]: comando ainda não suportado pelo gerador: '...'
     = dica: esta versão do Spike ainda não transforma esse comando em C.
```

## E902 — nenhum compilador C encontrado

**Mensagem:** `nenhum compilador C encontrado (procurei em SPIKE_CC e no PATH)` ou `SPIKE_CC aponta para '...', mas esse programa não foi encontrado`

**Quando acontece:** o Spike transforma o programa em C e precisa de um compilador C (GCC ou Clang) para gerar o executável. Ele procura primeiro na variável de ambiente `SPIKE_CC` e depois no `PATH`.

**Como corrigir:**
- No Linux: `sudo apt install build-essential` (ou instale o `clang`).
- No macOS: `xcode-select --install`.
- No Windows: instale o MinGW-w64 (por exemplo pelo MSYS2) e coloque a pasta `bin` dele no `PATH`, ou defina `SPIKE_CC` com o caminho do `gcc.exe`.
- Se você definiu `SPIKE_CC`, confira o caminho ou apague a variável.

```
spike ola.por
erro[E902]: nenhum compilador C encontrado (procurei em SPIKE_CC e no PATH)
```

## E903 — o compilador C recusou o código gerado

**Mensagem:** `o compilador C recusou o código gerado` (ou `o compilador C recusou o código gerado para a linha 7 de soma.por`)

**Quando acontece:** o C que o Spike gerou não compilou. Se o programa passou na análise do Spike, isso é sempre um defeito do Spike, nunca do seu programa. O C que vai para o compilador marca cada comando com a linha do `.por` de onde ele veio (`#line`), então o compilador C aponta para o seu arquivo. Quando isso acontece, a mensagem diz a linha e mostra o comando do `.por` que gerou o C recusado. Depois vêm as 20 primeiras linhas do que o compilador C reclamou.

**Como corrigir:** rode de novo com `--c` para guardar o arquivo C e abra uma issue em https://github.com/joaopssx/spike-compiler/issues com o `.por` e o `.c`.

```
spike programa.por --c
erro[E903]: o compilador C recusou o código gerado
```

## E904 — não foi possível gravar um arquivo

**Mensagem:** `não foi possível gravar '...'`

**Quando acontece:** o Spike não conseguiu criar o arquivo C temporário, a pasta temporária ou o executável. Em geral é falta de permissão na pasta, disco cheio, ou o executável ainda aberto de uma execução anterior (no Windows, um programa rodando não pode ser sobrescrito).

**Como corrigir:** feche o programa se ele ainda estiver rodando, confira se você pode escrever na pasta e se há espaço em disco.

```
spike jogo.por
erro[E904]: não foi possível gravar 'jogo.exe'
```

## E905 — download com SHA-256 errado

**Mensagem:** `o arquivo baixado não confere com o SHA-256 esperado e foi apagado`

**Quando acontece:** o `spike setup` baixou o compilador, mas a "impressão digital" (SHA-256) do arquivo não é a que está gravada no código do Spike. O arquivo pode ter chegado incompleto ou alterado no caminho. O Spike apaga o arquivo e não executa nada dele. A dica mostra o hash esperado e o recebido.

**Como corrigir:** rode `spike setup` de novo. Se o erro se repetir, um proxy da rede ou um antivírus pode estar alterando o download: tente em outra rede ou peça ajuda a quem cuida dela.

```
spike setup --yes
erro[E905]: o arquivo baixado não confere com o SHA-256 esperado e foi apagado
```

## E906 — falha ao baixar

**Mensagem:** `não consegui baixar: <motivo>; verifique a conexão, proxy ou antivírus`

**Quando acontece:** o `spike setup` não conseguiu baixar o compilador: sem internet, o endereço não foi encontrado, a conexão caiu, o servidor respondeu com erro ou o certificado HTTPS não é válido. Nada foi instalado.

**Como corrigir:** confira a internet e rode `spike setup` de novo. Em redes de escola ou empresa, um proxy pode bloquear o download; tente em outra rede. O Spike nunca desliga a verificação do certificado.

```
spike setup --yes
erro[E906]: não consegui baixar: o endereço do servidor não foi encontrado; verifique a conexão, proxy ou antivírus
```

## E907 — compilador baixado não funcionou

**Mensagem:** `não consegui extrair o compilador: <motivo>` (ou `o gcc.exe não apareceu ... depois de extrair`, ou `o gcc baixado não rodou ('gcc --version' falhou)`)

**Quando acontece:** o download deu certo e o arquivo conferiu, mas a extração falhou ou o `gcc.exe` extraído não rodou. Um antivírus que bloqueia programas recém-baixados é a causa mais comum.

**Como corrigir:** libere a pasta `%LOCALAPPDATA%\Spike` no antivírus e rode `spike setup --force` para instalar de novo.

```
spike setup --force
erro[E907]: o gcc baixado não rodou ('gcc --version' falhou)
```

---

# Avisos

Um aviso (`aviso[W001]`, em amarelo) aponta algo que provavelmente não é o que você quis, mas que não impede o programa de compilar. Os avisos só aparecem quando o programa não tem nenhum erro, e saem antes do `ok:`.

- `--no-warnings` esconde os avisos.
- `--warnings-as-errors` faz qualquer aviso parar a compilação, com código de saída 1.

## W001 — variável nunca usada

**Mensagem:** `a variável 'idade' foi declarada, mas nunca é usada`

**Quando acontece:** a variável aparece no `var`, mas o programa nunca guarda nem lê um valor nela. Parâmetros de procedimentos e funções não entram nesta conta.

**Como corrigir:** apague a declaração, ou use a variável onde ela era necessária.

```
algoritmo "exemplo"
var
  idade : inteiro
inicio
  escreval("oi")
fimalgoritmo
```

## W002 — variável lida, mas sem valor

**Mensagem:** `a variável 'total' é lida, mas nunca recebe valor`

**Quando acontece:** o programa usa o valor da variável, mas em nenhum lugar guarda algo nela (com `<-`, `leia` ou um parâmetro `var`). Ela vai valer sempre o valor padrão: 0, `""` ou `falso`.

**Como corrigir:** guarde um valor antes de usar, por exemplo `total <- 0` ou `leia(total)`.

```
algoritmo "exemplo"
var
  total : inteiro
inicio
  escreval(total)
fimalgoritmo
```

## W003 — variável recebe valor, mas nunca é lida

**Mensagem:** `a variável 'resposta' recebe valor, mas nunca é lida`

**Quando acontece:** o programa guarda um valor na variável (com `<-` ou `leia`), mas nunca usa esse valor. A variável do `para` conta como lida, porque o próprio laço a usa.

**Como corrigir:** mostre o valor com `escreva`, use-o numa conta, ou apague a variável.

```
algoritmo "exemplo"
var
  resposta : inteiro
inicio
  resposta <- 42
fimalgoritmo
```

## W004 — variável do 'para' alterada dentro do laço

**Mensagem:** `a variável 'i' do 'para' é alterada dentro do laço`

**Quando acontece:** dentro do `para`, um `<-` ou `leia` muda a variável que o próprio `para` controla. Isso é permitido, mas muda quantas voltas o laço dá e quase sempre é engano.

**Como corrigir:** use outra variável para a conta. Se o laço precisa pular valores, use `passo` ou troque o `para` por um `enquanto`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  para i de 1 ate 10 faca
    i <- i + 1
  fimpara
fimalgoritmo
```

## W005 — variável local com o nome de uma global

**Mensagem:** `a variável local 'nome' tem o mesmo nome de uma variável global`

**Quando acontece:** um procedimento ou função declara no seu `var` uma variável com o mesmo nome de uma variável (ou constante) global. Dentro do subprograma, o nome passa a ser a local, e a global fica escondida. Parâmetros com o nome de uma global, como `fatorial(n)`, não dão aviso.

**Como corrigir:** se queria usar a global, apague a declaração local. Se queria uma variável nova, dê outro nome a ela.

```
algoritmo "exemplo"
var
  nome : caractere
procedimento saudacao()
var
  nome : caractere
inicio
  nome <- "Ana"
  escreval(nome)
fimprocedimento
inicio
  saudacao()
fimalgoritmo
```

## W006 — função pode terminar sem 'retorne'

**Mensagem:** `a função 'sinal' pode terminar sem 'retorne'`

**Quando acontece:** existe um caminho pela função que chega ao `fimfuncao` sem passar por um `retorne`. Se o programa seguir esse caminho, ele para com o erro de execução R009.

**Como corrigir:** coloque um `retorne` no fim da função, ou um `senao` com `retorne` em cada `se` que decide o resultado.

```
algoritmo "exemplo"
funcao sinal(n: inteiro): inteiro
inicio
  se n > 0 entao
    retorne 1
  fimse
fimfuncao
inicio
  escreval(sinal(5))
fimalgoritmo
```

## W007 — divisão por zero escrita no código

**Mensagem:** `divisão por zero` (ou `divisão por zero no 'div'`, ou `resto ('mod') de uma divisão por zero`)

**Quando acontece:** o divisor de um `/`, `div` ou `mod` é o número 0 escrito direto no programa. Quando essa conta rodar, o programa para com o erro de execução R001.

**Como corrigir:** troque o 0 pelo valor certo.

```
algoritmo "exemplo"
var
  x : inteiro
inicio
  x <- 10
  escreval(x div 0)
fimalgoritmo
```

## W008 — condição sempre verdadeira ou sempre falsa

**Mensagem:** `a condição do 'se' é sempre falsa` (ou do `'enquanto'`, ou do `'ate'`; ou `sempre verdadeira`)

**Quando acontece:** a condição de um `se`, `enquanto` ou `repita ... ate` é o próprio `verdadeiro` ou `falso`. Um `se falso` nunca roda, um `enquanto falso` nunca entra no laço, um `repita ... ate verdadeiro` roda uma vez só e um `enquanto verdadeiro` nunca termina. O `enquanto verdadeiro` (e o `repita ... ate falso`) que tem um `interrompa` ou um `retorne` dentro não dá aviso, porque esse é o jeito comum de escrever um laço que sai pelo meio.

**Como corrigir:** escreva a condição que você queria testar, ou tire o comando se ele não serve.

```
algoritmo "exemplo"
inicio
  se falso entao
    escreval("nunca")
  fimse
fimalgoritmo
```

## W009 — código que nunca roda

**Mensagem:** `este código nunca vai rodar`

**Quando acontece:** há comandos logo depois de um `retorne` ou de um `interrompa`, no mesmo bloco. O `retorne` sai do subprograma e o `interrompa` sai do laço, então os comandos seguintes nunca são executados. O aviso aparece uma vez por bloco, no primeiro comando que não roda.

**Como corrigir:** apague esses comandos, ou mova-os para antes do `retorne` ou do `interrompa`. Em geral o `interrompa` deveria estar dentro de um `se`.

```
algoritmo "exemplo"
var
  i : inteiro
inicio
  i <- 0
  enquanto i < 10 faca
    interrompa
    i <- i + 1
  fimenquanto
fimalgoritmo
```

## W010 — arquivo em Windows-1252

**Mensagem:** `o arquivo estava em Windows-1252 e foi convertido para UTF-8; salve como UTF-8 para evitar problemas`

**Quando acontece:** o arquivo `.por` foi salvo na codificação antiga do Windows (aparece como "ANSI" no Bloco de Notas e como "Windows 1252" no VS Code), não em UTF-8. O Spike percebe isso pelos acentos, converte o texto e compila normalmente. A seta aponta o primeiro caractere convertido.

**Como corrigir:** salve o arquivo como UTF-8.

- Bloco de Notas: Arquivo > Salvar como, e em "Codificação" escolha UTF-8.
- VS Code: clique em "Windows 1252" na barra de baixo, escolha "Salvar com Codificação" e depois UTF-8.
- Notepad++: menu Codificação > Converter para UTF-8, e salve.

Se o arquivo tiver bytes que nem em Windows-1252 formam texto (como caracteres de controle), o erro é o E005.

Este programa, salvo pelo Bloco de Notas como "ANSI", compila com o aviso W010 apontando para o `ç`:

```
algoritmo "exemplo"
inicio
  escreval("Situação")
fimalgoritmo
```
