# Spike

[![Build](https://img.shields.io/github/actions/workflow/status/joaopssx/spike/build.yml?branch=main&label=build)](https://github.com/joaopssx/spike/actions)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](CMakeLists.txt)
[![Site](https://img.shields.io/badge/site-GitHub%20Pages-2ea44f)](https://joaopssx.github.io/spike/)

Spike e um compilador moderno de Portugol escrito em C++ que transpila Portugol para C e gera executaveis nativos.

Criado por Joao Pedro de Sena Santana. O projeto foi nomeado em homenagem ao cachorro do autor, Spike.

Site oficial: `https://joaopssx.github.io/spike/`

## Recursos

- arquitetura moderna de compilador em C++ com `std::unique_ptr`, `std::vector` e `std::unordered_map`
- lexer e parser recursive descent
- pipeline baseado em AST com analise semantica
- transpilacao de Portugol para C
- geracao de executaveis nativos com `gcc`, `clang` ou outro compilador C compativel
- CLI amigavel para debug, com inspecao de tokens e AST

## MVP Suportado

- `algoritmo`, `var`, `inicio`, `fimalgoritmo`
- tipos `inteiro` e `real`
- atribuicao com `<-`
- `escreva(...)` e `leia(...)`
- `se / senao / fimse`
- `enquanto / faca / fimenquanto`
- operadores aritmeticos e relacionais

## Exemplo

Entrada em Portugol:

```portugol
algoritmo "hello"

var
    a: inteiro

inicio
    a <- 10
    escreva(a)
fimalgoritmo
```

Execute com o Spike:

```bash
./build/spike run examples/hello.por
```

Saida compilada:

```text
10
```

## Estrutura do Projeto

```text
spike/
├── include/
├── src/
├── examples/
├── docs/
├── docs_pt/
├── website/
├── tests/
├── scripts/
├── .github/workflows/
├── LICENSE
├── CHANGELOG.md
├── CMakeLists.txt
├── README.md
└── README.pt-BR.md
```

## Instalacao

Linux / macOS:

```bash
git clone https://github.com/joaopssx/spike
cd spike
cmake -S . -B build
cmake --build build
```

Windows:

```bash
git clone https://github.com/joaopssx/spike
cd spike
cmake -S . -B build
cmake --build build --config Release
```

## Uso

Fluxo padrao de compilacao:

```bash
./build/spike examples/hello.por
```

Subcomandos principais:

```bash
./build/spike build examples/hello.por
./build/spike run examples/hello.por
./build/spike tokens examples/hello.por
./build/spike ast examples/if.por
./build/spike emit-c examples/loop.por --emit-c generated/loop.c
```

Flags:

```text
--tokens
--ast
--emit-c [arquivo.c]
--no-build
--run
--cc <compilador>
-o <binario>
```

## Documentacao

- `docs/` - documentacao em ingles
- `docs_pt/` - documentacao completa em portugues
- `docs_pt/grammar.md` - gramatica da linguagem e precedencia
- `docs_pt/architecture.md` - pipeline do compilador e visao dos modulos
- `docs_pt/tutorial.md` - guia rapido com exemplos
- `docs_pt/roadmap.md` - roadmap publico
- `docs_pt/contributing.md` - guia de contribuicao
- `docs_pt/changelog.md` - historico detalhado de releases

## Site e GitHub Pages

- o site estatico fica em `website/`
- o deploy automatico esta em `.github/workflows/pages.yml`
- depois de ativar o GitHub Pages com GitHub Actions, o site fica disponivel em `https://joaopssx.github.io/spike/`
- o site e atualizado automaticamente a cada push na branch `main`
- ingles e o idioma padrao do site, com um botao no cabecalho para alternar para portugues

Paginas principais:

- `website/index.html` - pagina inicial
- `website/docs.html` - panorama da documentacao
- `website/tutorial.html` - tutorial completo do projeto

## CI e Testes

- workflow do GitHub Actions em `.github/workflows/build.yml`
- deploy do site em `.github/workflows/pages.yml`
- smoke tests com CMake/CTest definidos em `CMakeLists.txt`
- comando local de teste:

```bash
ctest --test-dir build --output-on-failure
```

## Autor

Joao Pedro de Sena Santana

## Licenca

MIT License. Veja `LICENSE`.
