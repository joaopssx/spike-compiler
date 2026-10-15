# The Spike language

This is the reference for the Portugol dialect that Spike compiles. When the code and this file disagree, one of them is a bug.

Sections marked **proposed** describe syntax that does not exist yet. They are written down so they can be confirmed before anyone implements them.

## Overview

- Spike is for students taking their first programming course. When a rule could go either way, it goes the way that surprises a beginner less.
- The dialect follows VisuAlg (`algoritmo`, `var`, `inicio`, `<-`, `fimalgoritmo`). Where Spike differs or adds something, this file says so.
- A program compiles to a standalone executable. Spike ships with its own C compiler, so the student does not install anything else.
- Windows, Linux and macOS get the same level of support: same features, installer and tests on all three.
- Integers are 64 bits. `/` always produces a real number (`7 / 2` is `3.5`); `div` is integer division.
- Keywords and names ignore case. Names use ASCII letters only.

## Program structure

```
algoritmo "nome do programa"
var
  declarations
inicio
  commands
fimalgoritmo
```

- The program name is a string in double quotes.
- The `var` section is optional. A program with no variables goes straight from `algoritmo` to `inicio`.
- The body can be empty.
- Nothing may follow `fimalgoritmo` except comments and blank lines.

## Comments

- `//` starts a comment that runs to the end of the line.
- `{ ... }` is a block comment. It can span several lines and does not nest. A `{` that is never closed is error E004.
- Comments can appear anywhere a space can, including in the middle of a line.

## Identifiers

- A name starts with an ASCII letter or `_`, followed by ASCII letters, digits or `_`.
- Letters with accents (`média`, `ação`) are not allowed. They produce E001, and the hint suggests writing the name without the accent.
- Names ignore case: `Idade`, `idade` and `IDADE` are the same variable. Error messages show the name as it was declared.
- A name cannot be a keyword. Keywords also ignore case, and `caracter` is accepted as a spelling of `caractere`.
- Each name is declared once. Declaring the same name twice is an error (checked from v0.2).

## Types

| Type | Holds | Size | Default value |
| --- | --- | --- | --- |
| `inteiro` | whole numbers | 64-bit signed | `0` |
| `real` | decimal numbers | 64-bit floating point (C `double`) | `0.0` |
| `caractere` | text of any length | UTF-8 bytes | `""` |
| `logico` | `verdadeiro` or `falso` | | `falso` |

- Every variable starts with its default value, so a program never reads garbage.
- Reading a variable before the program assigns it is still almost always a mistake. The compiler warns about it (from v0.3); it is not an error.
- Integer overflow is not checked yet. The result wraps around, as in C.

## Literals

- Integer: digits only, `42`. A literal that does not fit in 64 bits is a compile error.
- Real: digits, a point, digits: `3.14`. `3.` is error E003. There is no exponent notation (`1e5`).
- Negative numbers are written with unary minus: `-5`.
- Text: double quotes on a single line, `"Olá"`. Escapes: `\"`, `\\`, `\n`, `\t`. An unknown escape such as `\q` is kept as written (backslash plus letter). A string that reaches the end of the line is error E002.
- Logical: `verdadeiro` and `falso`, in any case.

## Operators and precedence

From the loosest binding to the tightest:

| Level | Operators | Operands | Result |
| --- | --- | --- | --- |
| 1 | `ou` | logico | logico |
| 2 | `e` | logico | logico |
| 3 | `nao` (unary) | logico | logico |
| 4 | `=` `<>` `<` `>` `<=` `>=` | two numbers, or two texts, or two logicos (`=` `<>` only) | logico |
| 5 | `+` `-` | numbers; `+` also joins two texts | see below |
| 6 | `*` `/` `div` `mod` | numbers (`div`, `mod`: inteiro only) | see below |
| 7 | `^` (proposed) | numbers | real |
| 8 | `-` (unary) | number | same type |
| 9 | literals, names, `( )`, function calls | | |

- Binary operators group left to right, except `^`, which groups right to left (`2 ^ 3 ^ 2` is `2 ^ 9`).
- `+`, `-`, `*` on two `inteiro` give `inteiro`; if either side is `real`, the result is `real`.
- `/` always gives `real`. `div` and `mod` take two `inteiro` and give `inteiro`; `mod` keeps the sign of the left side, as in C.
- `^` always gives `real`, like `pot`.
- `e` and `ou` stop early: `falso e x` does not evaluate `x`.
- Texts compare alphabetically, byte by byte.
- `-2 ^ 2` is `-(2 ^ 2)`, which is `-4`.

## Conversions

- `inteiro` to `real` is automatic: `x <- 5` works when `x` is `real`, and `2 + 0.5` is `2.5`.
- `real` to `inteiro` is never automatic. `i <- 3.7` is an error; the hint points to `trunca` or `arredonda`.
- Text and numbers never mix. `"idade: " + 18` is an error; use `paraTexto(18)` or pass both to `escreva`, which takes several values.
- `+` between two texts joins them: `"Olá, " + nome`.
- `logico` does not convert to or from anything.

## Commands available today

### Assignment

`nome <- expressão` or `nome := expressão`. The value must have the variable's type, apart from the `inteiro` to `real` rule above.

### escreva and escreval

`escreva(a, b, ...)` writes each value in order, with no separator. `escreval` does the same and then starts a new line. `escreval()` only starts a new line.

- `inteiro` is written in full: `42`, `-7`.
- `real` is written in its shortest form that keeps the value, always with a decimal point: `3.5`, `2.0`, `0.1`.
- `logico` is written as `verdadeiro` or `falso`.

### leia

`leia(a, b, ...)` reads one line from the keyboard for each variable, in order.

- `inteiro` and `real` accept spaces around the number. `real` accepts `3.5` and `3,5`.
- `caractere` takes the whole line as typed.
- `logico` accepts `verdadeiro` or `falso`, in any case.
- Anything else is runtime error R003.

### se

```
se condição entao
  ...
senao
  ...
fimse
```

The condition must be `logico`. `senao` is optional.

### enquanto

```
enquanto condição faca
  ...
fimenquanto
```

The condition is tested before each pass.

### para

```
para i de 1 ate 10 passo 2 faca
  ...
fimpara
```

- The counter must be an `inteiro` variable. The start, end and step must be `inteiro`.
- Start, end and step are evaluated once, before the loop begins.
- Without `passo`, the step is 1. With a positive step the loop runs while `i <= fim`; with a negative step, while `i >= fim`. A step of 0 is runtime error R006.
- Assigning to the counter inside the loop is an error (checked from v0.2).
- After the loop, the counter holds the first value that failed the test (`11` in the example above).

### repita

```
repita
  ...
ate condição
```

The body runs at least once. The loop stops when the condition becomes `verdadeiro`.

## Scope

- Today every variable is global: it is declared in `var` and can be used anywhere in the body.
- Using a name that was not declared is an error.
- When subprograms arrive (v0.3), they get their own `var` section. A local name hides a global with the same name inside that subprogram.

## Language roadmap (proposed)

Each item below is **proposed**. The syntax is final only when the feature ships, in v0.3.

| Feature | Proposed syntax |
| --- | --- |
| Extra operators | `a div b`, `a mod b`, `a ^ b` |
| Constants | `const` section before `var`, lines `NOME = valor` |
| Break | `interrompa` inside `enquanto`, `para`, `repita` |
| Multiple choice | `escolha` / `caso` / `outrocaso` / `fimescolha` |
| Vectors | `v : vetor[1..10] de inteiro` |
| Matrices | `m : vetor[1..3, 1..3] de real` |
| Procedures | `procedimento nome(...)` ... `fimprocedimento` |
| Functions | `funcao nome(...): tipo` ... `retorne x` ... `fimfuncao` |
| By-reference parameters | `var` before a parameter |
| Initial value | `x : inteiro <- 5` in the `var` section |

```
algoritmo "roadmap"
const
  PI = 3.14159
  MAX = 10
var
  notas : vetor[1..MAX] de real
  tabela : vetor[1..3, 1..3] de inteiro
  total : inteiro <- 0

procedimento troca(var a, b: inteiro)
var
  aux : inteiro
inicio
  aux <- a
  a <- b
  b <- aux
fimprocedimento

funcao area(raio: real): real
inicio
  retorne PI * raio ^ 2
fimfuncao

inicio
  escolha total
  caso 1
    escreval("um")
  caso 2, 3
    escreval("dois ou três")
  outrocaso
    escreval("outro")
  fimescolha
  enquanto verdadeiro faca
    total <- total + 1
    se total mod 7 = 0 entao
      interrompa
    fimse
  fimenquanto
fimalgoritmo
```

- Parameters follow VisuAlg: names of the same type share a group (`a, b: inteiro`) and groups are separated by `;` (`a: inteiro; var b: real`). `var` before a group makes those parameters by reference.
- Subprograms are declared between the `var` section and the main `inicio`.
- Vector indexes start at the lower bound written in the declaration. Indexing is checked at run time (R002).
- `caso` accepts a list of values of the same type as the `escolha` expression; `inteiro` and `caractere` are allowed.

## Built-in functions (proposed)

Names ignore case, like everything else. They arrive in v0.3.

| Function | Returns | Description |
| --- | --- | --- |
| `abs(x)` | same as `x` | absolute value |
| `raiz(x)` | real | square root; negative `x` is R005 |
| `pot(b, e)` | real | `b` raised to `e` |
| `arredonda(x)` | inteiro | nearest integer, halves away from zero |
| `trunca(x)` | inteiro | drops the decimal part |
| `piso(x)` | inteiro | largest integer not above `x` |
| `teto(x)` | inteiro | smallest integer not below `x` |
| `seno(x)`, `cosseno(x)`, `tangente(x)` | real | trigonometry, `x` in radians |
| `log(x)` | real | base-10 logarithm, as in VisuAlg; `x <= 0` is R005 |
| `exp(x)` | real | e raised to `x` (VisuAlg uses `exp` for power; Spike uses `pot`) |
| `aleatorio(a, b)` | inteiro | random integer from `a` to `b`, inclusive |
| `tamanho(t)` | inteiro | number of characters in `t` |
| `maiusculo(t)`, `minusculo(t)` | caractere | `t` in upper or lower case |
| `copia(t, inicio, n)` | caractere | `n` characters of `t` starting at position `inicio` (first is 1) |
| `pos(sub, t)` | inteiro | position of `sub` in `t`, or 0 if absent |
| `concatena(a, b)` | caractere | same as `a + b` |
| `paraTexto(x)` | caractere | number or logico as text |
| `paraInteiro(t)` | inteiro | text to integer; invalid text is R004 |
| `paraReal(t)` | real | text to real; invalid text is R004 |
| `ord(c)` | inteiro | code of the first character of `c` |
| `chr(n)` | caractere | character with code `n` |

## Runtime errors

A compiled program stops at the first runtime error. It prints a message in Portuguese with the line of the `.por` file where the error happened, and exits with code 101.

| Code | When |
| --- | --- |
| R001 | division by zero (`/`, `div`, `mod`) |
| R002 | vector index outside the declared range |
| R003 | `leia` got input that does not match the variable's type |
| R004 | `paraInteiro` or `paraReal` got invalid text |
| R005 | math function outside its domain (`raiz` of a negative, `log` of zero or less) |
| R006 | `para` with a step of 0 |

The numbering is proposed and becomes final in v0.3.
