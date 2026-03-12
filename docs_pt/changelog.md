# Changelog Detalhado

## v2.0.0 (planejado)

### Objetivos

- deixar o Spike claramente compiler-first de novo, tanto na estrutura quanto nas estatisticas de linguagem do GitHub
- expandir de forma forte a base principal de C++ em `src/` e `include/`
- introduzir modos avancados de instrumentacao e debug do compilador

### Planejado

- ajuste do GitHub Linguist via `.gitattributes`
- flags de tempo, trace e estatisticas
- exportacao da AST em JSON e em Graphviz
- comandos `doctor` e `self-test`
- mais testes e utilitarios focados no compilador em si

## v0.1.0

Estrutura inicial open-source do Spike.

### Adicionado

- lexer com modo de inspecao de tokens
- parser recursive descent com modo de dump da AST
- analise semantica para declaracoes, atribuicoes e condicoes
- gerador de codigo C para o subconjunto MVP de Portugol
- build nativo automatico via compilador C do sistema
- projeto com CMake, exemplos, docs, site base e workflow do GitHub Actions

### Observacoes

- o MVP e intencionalmente pequeno e focado em clareza educativa
- futuras versoes vao expandir a linguagem em etapas controladas
