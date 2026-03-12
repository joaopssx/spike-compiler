# Tutorial do Spike

Este tutorial mostra o caminho mais rapido para compilar e inspecionar um programa em Portugol com o Spike.

## 1. Compilar o Spike

```bash
cmake -S . -B build
cmake --build build
```

## 2. Exemplo Hello

Crie `hello.por`:

```portugol
algoritmo "hello"

var
    a: inteiro

inicio
    a <- 10
    escreva(a)
fimalgoritmo
```

Compile:

```bash
./build/spike build hello.por
```

Compile e execute:

```bash
./build/spike run hello.por
```

## 3. Inspecionar Tokens

```bash
./build/spike tokens hello.por
```

Estilo esperado da saida:

```text
Algoritmo -> algoritmo
String -> hello
Var -> var
Identifier -> a
```

## 4. Inspecionar a AST

```bash
./build/spike ast hello.por
```

## 5. Gerar Apenas C

```bash
./build/spike emit-c hello.por --emit-c generated/hello.c
```

## 6. Exemplo com Fluxo de Controle

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

Execute:

```bash
./build/spike run examples/loop.por
```

## 7. Fluxo Recomendado

1. comece com `spike tokens` se o codigo parecer malformado
2. use `spike ast` para verificar a estrutura criada pelo parser
3. use `spike emit-c` para inspecionar o C gerado
4. use `spike run` quando o pipeline estiver correto
