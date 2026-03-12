# Arquitetura do Spike

O Spike e organizado como um pipeline classico de compilador educativo.

## Pipeline

```text
Codigo-Fonte
  -> Lexer
  -> Parser
  -> AST
  -> Analise Semantica
  -> Geracao de Codigo
  -> Codigo C
  -> Executavel Nativo
```

## Modulos

### Lexer

Arquivos: `include/token.hpp`, `include/lexer.hpp`, `src/lexer.cpp`

Responsabilidades:

- ler o codigo-fonte caractere por caractere
- emitir tokens com informacao de linha e coluna
- reconhecer palavras-chave, identificadores, numeros, strings e operadores

### Parser

Arquivos: `include/parser.hpp`, `src/parser.cpp`

Responsabilidades:

- consumir o fluxo de tokens
- implementar um parser recursive descent
- respeitar a precedencia dos operadores
- construir a AST

### AST

Arquivos: `include/ast.hpp`

Responsabilidades:

- representar programas Portugol em memoria
- armazenar declaracoes, comandos e expressoes
- manter metadados de tipo e localizacao para as etapas seguintes

### Analise Semantica

Arquivos: `include/semantic.hpp`, `src/semantic.cpp`

Responsabilidades:

- construir a tabela de simbolos
- rejeitar variaveis duplicadas ou nao declaradas
- inferir tipos de expressoes
- validar atribuicoes e condicoes

### Geracao de Codigo

Arquivos: `include/codegen.hpp`, `src/codegen.cpp`

Responsabilidades:

- percorrer a AST recursivamente
- emitir codigo C valido
- mapear tipos de Portugol para tipos de C
- preservar a semantica das expressoes com parenteses explicitos

### CLI

Arquivo: `src/main.cpp`

Responsabilidades:

- interpretar comandos e flags do usuario
- executar as etapas do pipeline em ordem
- imprimir tokens ou AST quando solicitado
- chamar opcionalmente o compilador C externo

### Servicos Core

Arquivos: `include/core/source_manager.hpp`, `include/core/diagnostics.hpp`, `include/core/session.hpp`, `include/core/phase_profiler.hpp`

Responsabilidades:

- carregar o arquivo de entrada e expor linhas do codigo com `SourceManager`
- renderizar diagnosticos melhores com `DiagnosticEngine`
- centralizar o estado da compilacao com `CompilationSession`
- medir cada fase do pipeline com `PhaseProfiler`

### Camada de CLI

Arquivos: `include/cli/options.hpp`, `src/cli/options.cpp`

Responsabilidades:

- interpretar subcomandos como `build`, `run`, `tokens`, `ast`, `emit-c` e `trace`
- expor opcoes de tempo e rastreamento como `--time` e `--trace`
- manter o `main.cpp` focado no pipeline em vez de concentrar toda a logica de argumentos

## Objetivos de Design

- ser facil de ler para estudantes aprendendo compiladores
- usar seguranca de memoria moderna de C++ com `std::unique_ptr`
- manter fluxos pequenos e explicitos em vez de esconder a logica
- oferecer build portavel com CMake
