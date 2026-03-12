# Arquitetura

O Spike e dividido em fases do compilador e servicos centrais.

- `core/`: source management, diagnostics, session e profiling
- `lexer/`: tokenizacao
- `parser/`: parser recursive descent
- `ast/`: arvore sintatica em memoria e printers
- `sema/`: checagem de simbolos e tipos
- `backend/`: emissao de C e integracao com o build nativo

O design de longo prazo inclui IR e passes de otimizacao.
