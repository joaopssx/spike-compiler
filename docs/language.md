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
- Source files are UTF-8. A UTF-8 byte order mark is ignored, Windows line endings (`\r\n`, or a lone `\r`) are read as `\n`, and line and column numbers come out the same either way. A file saved as Windows-1252 ("ANSI") is converted to UTF-8 with warning W010; a file that is not text in either encoding is error E005.

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
- Parameters are passed by value: the procedure gets a copy, and changing it does not change the caller's variable. `var` before a parameter passes it by reference instead (see below). A parameter can be `inteiro`, `real`, `caractere` or `logico`; a whole vector cannot be passed yet (E788, see Future ideas).
- A procedure may have its own `var` section, between the header and its `inicio`. It closes with `fimprocedimento` (E832).
- A call is a command of its own: `saudar("Ana", 2)`, or `linha()` without arguments. Any name followed by `(` at the start of a command is read as a call; calling a name that is not a procedure is E780.
- The call needs one argument per parameter (E781), each of the parameter's type (E782). An `inteiro` argument is accepted for a `real` parameter.
- A procedure can call any other procedure, including one declared further down, and itself (recursion).
- Two procedures with the same name, or a procedure with the name of a global variable or constant, is E783. Using a procedure name as a value or a variable (`x <- saudar`) is E784.
- `interrompa` inside a procedure only leaves a loop that is also inside that procedure.
- `procedimento` and `fimprocedimento` are reserved words.
- In the generated C, a procedure becomes `static void p_saudar(spike_str v_nome, spike_int v_vezes)`. All prototypes come first, so the order of the definitions does not matter.

### Parameters by reference

```
procedimento troca(var a: inteiro, var b: inteiro)
var
  t : inteiro
inicio
  t <- a
  a <- b
  b <- t
fimprocedimento
...
troca(x, y)
```

- `var` before a parameter group passes those parameters by reference: the procedure or function works on the caller's own variable, so a change inside shows up outside. `var a, b: inteiro` marks both names; in `(var a: inteiro, b: inteiro)` only `a` is by reference. Parameters without `var` are still copies.
- The argument for a parameter by reference must be a variable or an element of a vector or matrix: `troca(x, y)`, `troca(v[i], v[i + 1])`. A literal, a calculation or a call is E785, and a constant is E786 (constants never change, and a `var` parameter could change it).
- The type must be exactly the parameter's type, with no conversion: an `inteiro` variable cannot go to a `real` parameter by reference (E787). Parameters by value still accept `inteiro` for `real`.
- Passing a variable by reference counts as assigning it, since the subprogram may change it.
- The same variable can go to two parameters by reference: `troca(x, x)` leaves `x` as it was, and each change through one name is seen through the other.
- An index is checked once, when the call starts: `troca(v[i], v[j])` with `i` outside the vector stops with R002 before the procedure runs.
- In the generated C, a parameter by reference is a pointer (`spike_int* v_a`), each use inside is `(*v_a)`, and the call passes the address: `p_troca(&v_x, &v_v[spike_index(...)])`.

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

### Order of evaluation

```
q <- divide(17, 5, resto) + resto
```

- An expression is evaluated from left to right, the way it is read. Above, `divide` runs first and changes `resto`, so the `resto` on the right is already the new value. In `resto + divide(17, 5, resto)` the `resto` on the left is read before the call and keeps the old value.
- The same holds for every place with more than one value: both sides of an operator, the arguments of a call (`f(a, g(a))` reads `a` before calling `g`), the indexes of a matrix, and an assignment, where the index of the target comes before the value (`v[i] <- avanca(i)` uses the `i` from before the call).
- `e` and `ou` still stop early: in `x > 0 e f(x) > 0`, `f` is not called when `x > 0` is false.
- The order matters only when a function called in the expression has an effect: it changes a global variable or a parameter by reference, reads with `leia` or writes with `escreva` (directly or through a subprogram it calls). `le() - le()` subtracts the second value read from the first.
- In C the order of the arguments of a call is not defined, so in the generated C Spike stores the parts that must come first in temporaries before the line that uses them:

```c
{
    spike_int spike_tmp_1 = f_divide(17LL, 5LL, &v_resto);
    v_q = spike_add_int(spike_tmp_1, v_resto);
}
```

- A temporary appears only where the order could change the result. `fib(n - 1) + fib(n - 2)` and `maximo(maximo(a, b), c)` stay on one line, since those functions change nothing outside themselves. A condition of `enquanto` or `repita` that needs temporaries becomes `while (1)` with the temporaries and an `if (...) break;`, so it is evaluated again on every turn. When the right side of `e` or `ou` needs them, the test goes into a `spike_bool` temporary and the right side runs inside an `if`.

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

- Parameters work as in procedures and functions today: names of the same type share a group (`a, b: inteiro`), groups are separated by commas, and `var` before a group makes those parameters by reference.

## Future ideas

Not planned for a specific version yet.

- Passing a whole vector or matrix to a procedure or function (`procedimento soma(v: vetor[1..10] de inteiro)`). Today the header is read, but it is error E788; the workaround is a global vector, which every subprogram can use.

## Built-in functions

Spike comes with ready-made functions. They are called like any function, `raiz(16)`, and work anywhere a value of their result type does. Their names ignore case, and a procedure or function of the program cannot reuse one (E789). The list lives in a single table (`src/builtins/builtins.cpp`) that both the analyzer and the C generator read.

### Math

| Function | Result | What it does |
| --- | --- | --- |
| `abs(inteiro)` / `abs(real)` | same type as the argument | absolute value |
| `raiz(real)` | real | square root; a negative argument is R005 |
| `pot(real, real)` | real | first number raised to the second, same as `^` (R005 when there is no real result) |
| `arredonda(real)` | inteiro | nearest integer; halves go away from zero: `2.5` gives `3`, `-2.5` gives `-3` |
| `trunca(real)` | inteiro | drops the decimal part: `-2.7` gives `-2` |
| `piso(real)` | inteiro | largest integer not above the number: `-2.1` gives `-3` |
| `teto(real)` | inteiro | smallest integer not below the number: `-2.9` gives `-2` |
| `seno(real)`, `cosseno(real)`, `tangente(real)` | real | trigonometry, angle in radians |
| `log(real)` | real | base-10 logarithm, as in VisuAlg; zero or less is R005 |
| `exp(real)` | real | e raised to the number (VisuAlg uses `exp` for power; Spike uses `pot`) |
| `aleatorio(inteiro, inteiro)` | inteiro | random integer from the first number to the second, both included; first greater than second is R010 |

- An `inteiro` argument is accepted where `real` is asked, as in assignments: `raiz(16)` works. When a function has two versions, like `abs`, the one with the exact types wins.
- `arredonda`, `trunca`, `piso` and `teto` stop with R004 when the result does not fit in an `inteiro` (more than about 9.2 × 10^18), or when the number is not a number at all.
- Calling with the wrong number of arguments is E781, and with an argument of the wrong type is E782; the hint shows the signature, such as `raiz(real): real`. Using the name without parentheses (`x <- raiz`) is E784.
- `aleatorio` starts from the clock, so each run gives different numbers. When the environment variable `SPIKE_SEED` holds a number, the program uses it as the seed and repeats the same sequence on any system. The tests use this.
- A built-in can be called as a command on its own line; the result is discarded.
- In the generated C, a call becomes the function named in the table: `spike_round(v_nota, 8)` for functions that can fail (they get the line), or the C library function when it behaves the same, like `sin(v_x)` and `fabs(v_x)`.

### Text and conversion

| Function | Result | What it does |
| --- | --- | --- |
| `tamanho(caractere)` | inteiro | number of characters: `tamanho("ação")` is 4 |
| `maiusculo(caractere)`, `minusculo(caractere)` | caractere | the text in upper or lower case, accented letters included: `maiusculo("ação")` is `"AÇÃO"` |
| `copia(caractere, inteiro, inteiro)` | caractere | `copia(texto, inicio, quantidade)`: `quantidade` characters starting at position `inicio` (the first is 1) |
| `pos(caractere, caractere)` | inteiro | `pos(procurado, texto)`: position of the first `procurado` in `texto`, or 0 when it is absent |
| `concatena(caractere, caractere)` | caractere | same as `a + b` |
| `paraTexto(inteiro)` / `paraTexto(real)` / `paraTexto(logico)` | caractere | the value as text, written exactly as `escreva` would: `paraTexto(2.5)` is `"2.5"`, `paraTexto(verdadeiro)` is `"verdadeiro"` |
| `paraInteiro(caractere)` | inteiro | text to integer; invalid text is R004 |
| `paraReal(caractere)` | real | text to real, with a point or a comma (`"3,5"`); invalid text is R004 |
| `ehNumero(caractere)` | logico | whether `paraInteiro` or `paraReal` would accept the text |
| `ord(caractere)` | inteiro | code (Unicode code point) of the first character: `ord("A")` is 65, `ord("ç")` is 231; an empty text is R011 |
| `chr(inteiro)` | caractere | the character with that code; outside 1 to 1114111 is R011 |

- Texts are UTF-8, and these functions count characters, not bytes: `ç` is one character even though it takes two bytes. A byte that is not valid UTF-8 counts as one character, so a broken text never makes them stop or skip.
- `maiusculo` and `minusculo` change ASCII letters and the accented letters of Latin languages (`à á â ã ä ç é ê í ó ô õ ú ü` and the others between U+00C0 and U+00FE). Other characters stay as they are.
- `copia` never fails because of the end of the text: it returns what fits, and an empty text when `inicio` is past the end or `quantidade` is 0 or negative. Only `inicio` below 1 is an error (R011).
- `pos` with an empty `procurado` returns 0.
- `paraInteiro` and `paraReal` accept spaces around the number, like `leia`. The R004 message shows the text: `não foi possível converter 'abc' para inteiro`.
- Each new text lives in memory owned by the runtime until the program ends; nothing needs to be freed by hand.

## Runtime errors

A compiled program stops at the first runtime error. It prints a message in Portuguese to stderr, in the same shape as a compile error: the code, the file and line, the line of the `.por` itself and a hint. When the error happens inside procedures or functions, up to 8 lines `chamado de 'nome' na linha N` follow, most recent first, then `... e mais N` for the rest. The program exits with code 101. The format and every hint are in `errors.md`.

| Code | When |
| --- | --- |
| R001 | division by zero (`/`, `div`, `mod`) |
| R002 | vector index outside the declared range |
| R003 | `leia` got input that does not match the variable's type |
| R004 | a conversion failed: `arredonda`, `trunca`, `piso` or `teto` got a number too big for `inteiro`, or `paraInteiro` or `paraReal` got invalid text |
| R005 | math result that is not a number or is infinite: `^` and `pot` (as in `0 ^ -1`), `raiz` of a negative, `log` of zero or less |
| R006 | `para` with a step of 0 |
| R007 | `leia` reached the end of the input |
| R008 | the program ran out of memory, or a recursion went too deep (more than 10000 calls in progress) |
| R009 | a function reached `fimfuncao` without `retorne` |
| R010 | `aleatorio` got a first number greater than the second |
| R011 | a text function got a position or code out of range: `copia` starting below 1, `ord` of an empty text, `chr` outside 1 to 1114111 |

The numbering is proposed and becomes final in v0.3.
