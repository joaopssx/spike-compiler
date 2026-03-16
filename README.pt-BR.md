# Spike Compiler

Projeto basico de um compilador de Portugol.

Spike e um compilador simples feito para estudo.
O primeiro objetivo e pequeno e claro: ler codigo Portugol, gerar C e compilar um programa nativo.

English version: `README.md`

## Objetivo

- Construir um compilador simples de Portugol.
- Gerar codigo C como saida.
- Usar `gcc` ou `clang` para criar o executavel.
- Manter o projeto pequeno no comeco.

## Primeira ideia de roadmap

O projeto cresce passo a passo:

- `0.x`: desenvolvimento inicial
- `1.0`: primeira versao estavel
- `1.x`: melhorias e ferramentas

## Pipeline inicial

```text
fonte.por
  -> lexer
  -> parser
  -> AST
  -> gerador C
  -> gcc/clang
  -> executavel
```

## Versao 0.1.0

Objetivo: ler um arquivo `.por` e reconhecer tokens.

Features planejadas:

- estrutura do projeto
- CLI basica
- leitura de arquivo `.por`
- lexer simples
- saida da lista de tokens
- comando `tokens`

Exemplo:

```bash
spike tokens arquivo.por
```

Tokens basicos desta versao:

- palavras-chave: `algoritmo`, `var`, `inicio`, `fimalgoritmo`, `inteiro`, `real`, `texto`, `leia`, `escreva`
- operadores: `+`, `-`, `*`, `/`, `<-`, `=`, `<`, `>`, `<=`, `>=`

Arquivos principais desta versao:

- `main`
- `lexer`
- `token`
- `token_type`

Esta versao ainda nao executa codigo.

## Versao 0.2.0

Objetivo: transformar tokens em uma AST basica.

Features planejadas:

- parser recursivo descendente
- AST basica
- statements simples
- comando `ast`

Suportado nesta versao:

- `var`
- atribuicao
- `escreva`

Exemplo:

```bash
spike ast arquivo.por
```

Saida de exemplo:

```text
Program
  VarDecl a : inteiro
  Assign a <- 10
  Write a
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Executar

```bash
./build/spike tokens arquivo.por
./build/spike ast arquivo.por
```

Exemplo:

```bash
./build/spike tokens examples/hello.por
./build/spike ast examples/ast-basic.por
```

## Notas

- Sem VM
- Sem LLVM
- Sem otimizacoes complexas
- Sem IDE
- Sem site
- Primeiro, so o essencial
