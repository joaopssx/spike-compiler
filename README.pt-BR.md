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
- nucleo do compilador dividido em `cli`, `core`, lexer, parser, analise semantica e modulos preparados para backend
- lexer e parser recursive descent
- pipeline baseado em AST com analise semantica
- transpilacao de Portugol para C
- geracao de executaveis nativos com `gcc`, `clang` ou outro compilador C compativel
- CLI amigavel para debug, com inspecao de tokens e AST
- medicao por fase e trace interno com `--time` e `--trace`
- saidas estruturadas com `spike stats`, `spike explain`, `spike ast --json` e `spike ast --graph`
- camada inicial de IR com `spike ir` e primeiro passe de otimizacao por constant folding

## Forma do Repositorio

- estrutura seria e compiler-first
- documentacao forte em ingles e portugues em `docs/en/` e `docs/pt/`
- templates de issue, workflows de CI e arquivos de contribuicao para o GitHub
- espaco claro para IR, passes, formatter, linter e watch mode sem virar bagunca

## Direcao da v2.0.0

- a v2.0.0 do Spike esta sendo preparada como uma release compiler-first
- o repositorio esta sendo ajustado para que as estatisticas de linguagem do GitHub reflitam o foco real do projeto: C++
- assets do site e da documentacao foram marcados como documentacao em `.gitattributes`, enquanto o nucleo em C++ vai crescer bastante nas proximas features
- a meta e ter uma participacao muito mais forte de C++, perto de 70% no breakdown de linguagens do repositorio

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
│   ├── cli/
│   ├── core/
│   └── ...
├── src/
│   ├── cli/
│   ├── core/
│   └── ...
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
./build/spike check examples/hello.por
./build/spike trace examples/hello.por
./build/spike tokens examples/hello.por
./build/spike ast examples/if.por
./build/spike ast examples/if.por --json --output generated/if.ast.json
./build/spike ast examples/if.por --graph --output generated/if.ast.dot
./build/spike ir examples/basic/hello.por
./build/spike ir examples/basic/hello.por --json --output generated/hello.ir.json
./build/spike stats examples/basic/hello.por
./build/spike explain examples/basic/hello.por
./build/spike emit-c examples/loop.por --emit-c generated/loop.c
./build/spike doctor
./build/spike version
```

Flags:

```text
--tokens
--ast
--emit-c [arquivo.c]
--no-build
--run
--json
--graph
--output <arquivo>
--stdout
--with-spans
--trace
--time
--cc <compilador>
-o <binario>
```

## Documentacao

- `docs/en/` - documentacao em ingles
- `docs/pt/` - documentacao em portugues
- `docs/en/cli.md` / `docs/pt/cli.md` - referencia de comandos
- `docs/en/architecture.md` / `docs/pt/architecture.md` - pipeline do compilador e modulos internos
- `docs/en/ir.md` / `docs/pt/ir.md` - camada de representacao intermediaria
- `docs/en/tutorial.md` / `docs/pt/tutorial.md` - uso guiado
- `docs/en/roadmap.md` / `docs/pt/roadmap.md` - roadmap do `v0.1.0` ao `v1.0.0`
- `docs/en/contributing.md` / `docs/pt/contributing.md` - guia de contribuicao
- `docs/index.md` - indice da documentacao

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
