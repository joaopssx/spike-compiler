# Roadmap do Spike

O Spike segue um roadmap incremental focado em entregar um compilador real antes de explorar recursos avancados.

## v0.1.0

- lexer
- parser recursive descent
- AST
- analise semantica
- transpilacao de Portugol para C
- CLI com `tokens`, `ast`, `build`, `run` e `emit-c`

## v0.2.0

- diagnosticos mais fortes com trechos de codigo mais ricos
- suporte melhor a `real`, `logico` e `caractere`
- mais verificacoes semanticas e cobertura de testes

## v0.3.0

- melhoria da superficie de entrada e saida da linguagem
- modo de exportacao da AST formatada ou em JSON
- artefatos de release para Linux, macOS e Windows

## v1.0.0

- especificacao estavel do MVP da linguagem
- documentacao e tutoriais refinados
- suite de testes automatizados forte
- experiencia de CLI pronta para uso publico

## v2.0.0

- foco compiler-first no repositorio, com GitHub Linguist configurado para priorizar C++ acima dos assets do site
- crescimento forte de C++ em `src/` e `include/`, mirando algo perto de 70% das estatisticas de linguagem do repositorio
- ferramental avancado do compilador: `--time`, `--trace`, `--stats`, `--ast-json`, `--graph`, `--doctor` e `--self-test`
- analise semantica mais profunda e instrumentacao interna mais rica para performance e debug
- mais testes automatizados focados em lexer, parser, analise semantica e geracao de codigo
- site mantido como camada leve de documentacao e apresentacao, sem dominar o projeto principal

## Ideias Futuras

- modo interpretador
- REPL
- formatador e linter
- extensao para VS Code
- backend LLVM
- alvo WebAssembly
