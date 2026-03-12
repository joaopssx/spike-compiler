# Roadmap

## v0.1.0 - Foundation

- estrutura inicial do projeto e base de CMake
- leitura de fonte, lexer, parser e AST
- analise semantica basica e geracao de C
- `spike build` e `spike run`

## v0.2.0 - Control Flow

- `se`, `senao`, `fimse`
- `enquanto`, `faca`, `fimenquanto`
- precedencia melhor de expressoes
- `spike tokens` e `spike ast`

## v0.3.0 - Semantic Analysis

- tabela de simbolos melhor
- checagem de variavel duplicada e nao declarada
- validacao de tipos em atribuicao e condicao
- `spike check`

## v0.4.0 - Better Tooling

- `spike emit-c`
- inspecao mais rica da AST
- base para JSON e Graphviz
- docs mais claras em portugues e ingles

## v0.5.0 - Diagnostics Upgrade

- diagnostic engine estruturado
- spans de fonte melhores
- saida com warning, note e help
- recovery melhor no parser

## v0.6.0 - IR Layer

- primeira camada de IR
- IR builder e printer
- separacao mais limpa entre frontend e backend

## v0.7.0 - Optimization Passes

- pass manager
- constant folding
- constant propagation
- base para dead code analysis

## v0.8.0 - Profiling and Advanced CLI

- `--time`
- `--mem`
- `--leaks`
- `spike doctor`
- `spike benchmark`

## v0.9.0 - Docs, Site and Cross-Platform Polish

- docs PT/EN mais fortes
- site mais refinado e screenshots
- CI em Linux, Windows e macOS
- preparacao para releases

## v0.9.5 - Ecosystem Tools

- formatter
- linter
- watch mode
- explain mode melhor

## v1.0.0 - Stable Core Release

- frontend, semantica e backend C estaveis
- CLI e diagnosticos solidos
- documentacao e exemplos completos
- releases prontas para portfolio e apresentacao publica
