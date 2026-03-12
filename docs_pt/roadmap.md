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

## Ideias Futuras

- modo interpretador
- REPL
- formatador e linter
- extensao para VS Code
- backend LLVM
- alvo WebAssembly
