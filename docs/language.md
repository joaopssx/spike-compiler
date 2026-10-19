# The Spike language

This is the reference for the Portugol dialect that Spike compiles. When the code and this file disagree, one of them is a bug.

Sections marked **proposed** describe syntax that does not exist yet. They are written down so they can be confirmed before anyone implements them.

## Overview

- Spike is for students taking their first programming course. When a rule could go either way, it goes the way that surprises a beginner less.
- The dialect follows VisuAlg (`algoritmo`, `var`, `inicio`, `<-`, `fimalgoritmo`). Where Spike differs or adds something, this file says so.
- A program compiles to a standalone executable. Spike turns it into C and calls a C compiler (GCC or Clang). Today that compiler has to be installed; from v0.4 Spike will ship with its own, so the student installs nothing else.
- Windows, Linux and macOS get the same level of support: same features, installer and tests on all three.
- Integers are 64 bits. `/` always produces a real number (`7 / 2` is `3.5`); `div` is integer division.
- Keywords and names ignore case. Names use ASCII letters only.

## Program structure

```
algoritmo "nome do programa"
var
  declarations
procedures
inicio
  commands
fimalgoritmo
```

- The program name is a string in double quotes.
- The `const` and `var` sections are optional; when both are present, `const` comes first. A program with neither goes straight from `algoritmo` to `inicio`.
- Procedures and functions, if any, come after `var` and before the main `inicio`.
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
| 7 | `-` (unary) | number | same type |
| 8 | `^` | numbers | real |
| 9 | literals, names, `( )`, function calls | | |

- Binary operators group left to right, except `^`, which groups right to left (`2 ^ 3 ^ 2` is `2 ^ 9`).
- `+`, `-`, `*` on two `inteiro` give `inteiro`; if either side is `real`, the result is `real`.
- `/` always gives `real`. `div` and `mod` take two `inteiro` and give `inteiro`; a `real` on either side is error E710. `div` cuts toward zero (`7 div 2` is `3`, `-7 div 2` is `-3`) and `mod` keeps the sign of the left side (`-7 mod 3` is `-1`, `7 mod -3` is `1`), as in C. A zero on the right of `div` or `mod` is runtime error R001.
- `^` always gives `real`, like `pot`: `2 ^ 10` is `1024` (printed without `.0`) and `2 ^ -1` is `0.5`. A result that is not a number or is infinite, such as `0 ^ -1` or `(-8) ^ 0.5`, is runtime error R005.
- `e` and `ou` stop early: `falso e x` does not evaluate `x`.
- Texts compare alphabetically, byte by byte.
- `-2 ^ 2` is `-(2 ^ 2)`, which is `-4`, and `2 * 3 ^ 2` is `18`. A minus right after `^` belongs to the exponent: `2 ^ -1`.
- `div` and `mod` are reserved words, so they cannot be used as variable names. Longer words that start with them, like `dividir` or `modo`, are still fine.

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
- `real` is written with up to 10 significant digits and no trailing zeros, as in VisuAlg: `3.5`, `0.1`, and `7` for `7.0`. The decimal separator is always a point, whatever the system language. Very large or very small values use exponent notation (`1e+20`).
- `logico` is written as `verdadeiro` or `falso`.

### leia

`leia(a, b, ...)` reads one line from the keyboard for each variable, in order. Two values on the same line (`3 4`) are not split: the first variable gets the whole line. One value per line keeps the rule simple and predictable.

Whatever was written with `escreva` before the `leia` appears on the screen before the program waits. A UTF-8 byte order mark at the very start of the input (added by Notepad and by PowerShell pipes) is ignored.

- `inteiro` and `real` accept spaces around the number. `real` accepts `3.5` and `3,5`.
- `caractere` takes the whole line as typed.
- `logico` accepts `verdadeiro`, `falso`, `v`, `f`, `sim` or `nao`, in any case.
- Anything else is runtime error R003, and the message shows what was typed: `esperava um número inteiro, mas recebeu 'abc'`. An empty line is also R003.
- If the input ends before a line could be read, it is R007, and the message names the variable: `a entrada acabou antes de leia() conseguir ler 'b'`.
- Output always uses a point for decimals, even when the input used a comma.

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
- Assigning to the counter inside the loop (with `<-` or `leia`) is allowed, as in VisuAlg, but it is almost always a mistake. The compiler warns about it (from v0.3).
- `passo 0` written as a literal is a compile error, because the loop would never end. A step that only turns out to be 0 while the program runs is runtime error R006.
- The end and the step are fixed when the loop starts. Changing the variables they came from inside the loop has no effect. Changing the counter itself does: the next pass continues from the new value.
- After the loop, the counter holds the first value that failed the test (`11` in the example above).
- If adding the step would go past the largest or smallest `inteiro`, the loop stops instead of wrapping around.

### repita

```
repita
  ...
ate condição
```

The body runs at least once. The loop stops when the condition becomes `verdadeiro`.

### escolha

```
escolha opcao
  caso 1
    escreval("um")
  caso 2, 3
    escreval("dois ou três")
  caso 4 ate 6
    escreval("de quatro a seis")
  outrocaso
    escreval("outro")
fimescolha
```

- The value after `escolha` is computed once and must be `inteiro` or `caractere` (E760).
- Each `caso` lists values separated by commas. For `inteiro`, a value can also be a range `inicio ate fim`, with the smaller number first (E764). Ranges do not work with texts (E765).
- Case values are literals or constants of the same type as the chosen value (E761, E763). A value that appears in two cases is an error (E762), including overlapping ranges.
- Only the first matching case runs; there is no falling through to the next one, unlike C's `switch`. If nothing matches, `outrocaso` runs; without `outrocaso`, nothing happens.
- `outrocaso` is optional and must be the last block (E812). A `caso` may have an empty body.
- `escolha` is not a loop: an `interrompa` inside it leaves the loop around the `escolha`.

### interrompa

```
enquanto verdadeiro faca
  leia(n)
  se n = 0 entao
    interrompa
  fimse
fimenquanto
```

`interrompa` leaves the innermost `enquanto`, `repita` or `para` right away; the program continues after its `fimenquanto`, `ate` or `fimpara`. In nested loops only the inner one stops. Inside a `se` it still refers to the loop around it. Outside any loop it is error E750. After `interrompa` in a `para`, the counter keeps the value it had.

## Vectors

```
const
  TOTAL = 5
var
  notas : vetor[1..TOTAL] de real
  nomes : vetor[0..9] de caractere
inicio
  notas[1] <- 8.5
  leia(notas[2])
  escreval(notas[1] + notas[2])
```

- `vetor[inicio..fim] de tipo` declares a numbered list of values of one type. The limits are integer literals or integer constants, may be negative (`vetor[-5..5]`), and the end cannot be smaller than the start (E775). A vector holds at most 10 million elements (E775).
- `notas[i]` is one element. It works anywhere a variable of the element type works: in expressions, on the left of `<-`, and in `leia`. The index must be `inteiro` (E772).
- A vector cannot be used whole, without an index, in an expression, an assignment, `escreva` or `leia` (E770), and it cannot be the counter of a `para` (E731). `[ ]` on something that is not a vector is E771.
- Every element starts with the default value of its type (`0`, `0.0`, `""`, `falso`).
- An index written with fixed values (`notas[6]`, `notas[TOTAL + 1]`) is checked while compiling (E774). Any other index is checked when the program runs: going outside the limits stops the program with runtime error R002, `índice 11 fora do vetor 'notas' (válido: 1 a 5)`.
- `vetor` is a reserved word. `..` is its own symbol, so `1..10` reads as `1`, `..`, `10`; a real number still needs a digit after the point (`1.5`).

### Matrices

```
var
  m : vetor[1..3, 1..4] de inteiro
  cubo : vetor[1..2, 1..2, 1..2] de real
inicio
  m[2, 3] <- 7
  escreval(m[2, 3], " ", cubo[1, 2, 1])
```

- A vector can have 2 or 3 dimensions: one `inicio..fim` pair per dimension, separated by commas. More than 3 is error E826, and a missing comma between pairs is E827.
- Each dimension follows the same rules as a one-dimensional vector. The limit of 10 million elements applies to the total (`linhas * colunas`), E775.
- Every use needs one index per dimension, `m[i, j]`. A different number of indices is E773: `'m' tem 2 dimensões, mas recebeu 1 índice`.
- Each index is checked against its own dimension. A fixed index outside it is E774 while compiling; any other stops the program with R002, `índice 5 fora da dimensão 2 de 'm' (válido: 1 a 4)`.
- In the generated C a matrix is one flat array in row order (`spike_int v_m[12];`), and `m[i, j]` reads the element at `(i - 1) * 4 + (j - 1)`.

## Constants

A `const` section, before `var`, gives a name to a value that never changes:

```
algoritmo "circulo"
const
  PI = 3.14159
  LIMITE = 10
  TITULO = "Área"
  AREA_MAXIMA = PI * LIMITE ^ 2
var
  r : real
inicio
  ...
fimalgoritmo
```

- One constant per line: `NOME = valor`. The section ends at `var` or `inicio`, and both `const` and `var` are optional.
- The type comes from the value: `inteiro`, `real`, `caractere` or `logico`.
- The value is made of literals, operators and constants declared on earlier lines. Using a variable, or a constant declared further down, is error E741.
- Spike computes the value while compiling, so the generated C reads `const spike_real v_pi = 3.14159;`. A value that cannot be computed, such as `1 div 0`, is error E742 instead of a runtime error.
- Constants and variables share names: the same name in both is error E701.
- Assigning to a constant, reading it with `leia` or using it as the counter of a `para` is error E740.
- `const` is a reserved word. Words that start with it, like `constante`, are still valid names.

## Procedures

```
algoritmo "demo"
var
  total : inteiro
procedimento saudar(nome: caractere, vezes: inteiro)
var
  i : inteiro
inicio
  para i de 1 ate vezes faca
    escreval("Oi, ", nome)
  fimpara
  total <- total + vezes
fimprocedimento
inicio
  saudar("Ana", 2)
fimalgoritmo
```

- A procedure is a named block of commands. It does not return a value. Procedures are declared after the program's `var` section and before the main `inicio`; one declared after it is error E833.
- The header is `procedimento nome(parâmetros)`. Parameters are `nome: tipo`, separated by commas; names of the same type can share a group: `(a, b: inteiro, c: real)`. A procedure without parameters still has the parentheses: `procedimento linha()`. A malformed header is E830 (no name) or E831.
- Parameters are passed by value: the procedure gets a copy, and changing it does not change the caller's variable. A parameter can be `inteiro`, `real`, `caractere` or `logico`; vectors cannot be passed yet (E831).
- A procedure may have its own `var` section, between the header and its `inicio`. It closes with `fimprocedimento` (E832).
- A call is a command of its own: `saudar("Ana", 2)`, or `linha()` without arguments. Any name followed by `(` at the start of a command is read as a call; calling a name that is not a procedure is E780.
- The call needs one argument per parameter (E781), each of the parameter's type (E782). An `inteiro` argument is accepted for a `real` parameter.
- A procedure can call any other procedure, including one declared further down, and itself (recursion).
- Two procedures with the same name, or a procedure with the name of a global variable or constant, is E783. Using a procedure name as a value or a variable (`x <- saudar`) is E784.
- `interrompa` inside a procedure only leaves a loop that is also inside that procedure.
- `procedimento` and `fimprocedimento` are reserved words.
- In the generated C, a procedure becomes `static void p_saudar(spike_str v_nome, spike_int v_vezes)`. All prototypes come first, so the order of the definitions does not matter.

## Functions

```
funcao fatorial(n: inteiro): inteiro
inicio
  se n <= 1 entao
    retorne 1
  fimse
  retorne n * fatorial(n - 1)
fimfuncao
```

- A function is a procedure that gives back a value. The header adds the type after the parameters: `funcao nome(parâmetros): tipo`, with `inteiro`, `real`, `caractere` or `logico`; a vector cannot be returned yet. A missing or invalid type is E835. The function closes with `fimfuncao` (E836). Parameters, locals and scope work exactly as in procedures.
- `retorne valor` ends the function and gives the value back. The value must have the function's type (E790); an `inteiro` is accepted in a `real` function. Inside a procedure, `retorne` alone ends the procedure early. `retorne` without a value in a function, or with a value in a procedure, is E793. Outside any function or procedure it is E791.
- The value of `retorne` is what follows it on the same line. A `retorne` with nothing after it on its line has no value.
- A call works anywhere a value of the function's type does: `escreval(fatorial(5))`, `se primo(n) entao`, `maximo(maximo(a, b), c)`. A function can also be called as a command on its own line; the value is then discarded.
- Functions and procedures share one set of names (E783). Calling a procedure inside an expression is E784, since it has no value; so is using a function name without parentheses (`x <- fatorial`).
- A function that may reach `fimfuncao` without passing through a `retorne` is not a compile error, because Spike cannot always tell whether that path really happens. If it happens while the program runs, the program stops with R009: `a função 'sinal' terminou sem retornar um valor`. The analyzer keeps the list of functions that may end that way, for a future warning.
- `funcao`, `fimfuncao` and `retorne` are reserved words.
- In the generated C, a function becomes `static spike_int f_fatorial(spike_int v_n)`, with the prototypes of every procedure and function first. Each `retorne` stores its value in a temporary, `spike_result`, then leaves.

### Recursion limit

- Every call to a function or procedure is counted while it runs. More than 10000 calls in progress at the same time stops the program with R008: `recursão profunda demais (limite de 10000 chamadas), verifique a condição de parada`. In practice this is a recursion that never reaches its stopping case.
- Spike also watches the memory the calls use. A recursion with large local vectors can stop before 10000 calls, with R008 and `a memória para chamadas acabou`. Either way the program stops with a message instead of crashing.
- On Windows the executable gets a 16 MB call stack, so 10000 ordinary calls fit. On Linux and macOS the system limit applies (usually 8 MB).

## Scope

- Variables declared in the program's `var` section are global: the main body and every procedure and function can use them.
- Parameters and the variables in the `var` section of a procedure or function are local: they exist only inside it, and every call starts them again with the default value of their type. Using a local name outside its procedure is E700.
- A local name may repeat a global one (variable or constant). Inside the procedure, the name refers to the local; outside, to the global. A future warning will point these out.
- Two names in the same scope are E701: two globals, two parameters, two locals, or a parameter and a local.
- A local vector holds at most 10 thousand elements (E775), since it lives on the stack while the procedure runs. Larger vectors belong in the program's `var` section.
- Using a name that was not declared is an error.

## Language roadmap (proposed)

Each item below is **proposed**. The syntax is final only when the feature ships, in v0.3.

| Feature | Proposed syntax |
| --- | --- |
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

- Parameters work as in procedures: names of the same type share a group (`a, b: inteiro`) and groups are separated by commas. `var` before a group makes those parameters by reference.

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
| R005 | math result that is not a number or is infinite: `^` (as in `0 ^ -1`), and later `raiz` of a negative or `log` of zero or less |
| R006 | `para` with a step of 0 |
| R007 | `leia` reached the end of the input |
| R008 | the program ran out of memory, or a recursion went too deep (more than 10000 calls in progress) |
| R009 | a function reached `fimfuncao` without `retorne` |

The numbering is proposed and becomes final in v0.3.
