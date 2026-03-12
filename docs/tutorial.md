# Spike Tutorial

This tutorial shows the fastest way to compile and inspect a Portugol program with Spike.

## 1. Build Spike

```bash
cmake -S . -B build
cmake --build build
```

## 2. Hello Example

Create `hello.por`:

```portugol
algoritmo "hello"

var
    a: inteiro

inicio
    a <- 10
    escreva(a)
fimalgoritmo
```

Compile it:

```bash
./build/spike build hello.por
```

Compile and run it:

```bash
./build/spike run hello.por
```

## 3. Inspect Tokens

```bash
./build/spike tokens hello.por
```

Expected style of output:

```text
Algoritmo -> algoritmo
String -> hello
Var -> var
Identifier -> a
```

## 4. Inspect the AST

```bash
./build/spike ast hello.por
```

## 5. Emit C Only

```bash
./build/spike emit-c hello.por --emit-c generated/hello.c
```

## 6. Control Flow Example

```portugol
algoritmo "loop"

var
    i: inteiro

inicio
    i <- 0
    enquanto i < 5 faca
        escreva(i)
        i <- i + 1
    fimenquanto
fimalgoritmo
```

Run it:

```bash
./build/spike run examples/loop.por
```

## 7. Recommended Workflow

1. start with `spike tokens` if the source looks malformed
2. use `spike ast` to verify the parser structure
3. use `spike emit-c` to inspect the generated C
4. use `spike run` once the pipeline looks correct
