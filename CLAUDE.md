# CLAUDE.md

## O que é o Spike

Spike é um compilador de Portugol para executável. Ele lê um arquivo `.por`, converte o programa para C e chama um compilador C (GCC ou Clang) para gerar o executável. Repositório: github.com/joaopssx/spike-compiler. Linguagem do compilador: C++17. Build: CMake. Autor: JP, estudante de Análise e Desenvolvimento de Sistemas. O alvo principal é Windows; Linux e macOS também precisam funcionar. O desenvolvimento acontece primeiro no Windows (MSYS2 UCRT64) e depois é conferido no Debian.

Público: estudantes brasileiros de introdução à programação.

Três diferenciais que devem ser protegidos em toda decisão:

1. Gera um executável independente (`.exe`) que roda sem o Spike instalado.
2. Mensagens de erro em português, mostrando a linha do código, uma seta no ponto exato e uma dica de correção.
3. O aluno pode ver o C gerado (flag `--c`) e aprender com ele.

## Pipeline

```
arquivo .por -> Lexer -> tokens -> Parser -> AST -> Analisador semântico -> Gerador de C -> compilador C -> executável
```

Hoje só existem o Lexer e o Parser.

## Estrutura de pastas e alvos do CMake

Esta seção descreve o estado atual. Atualize-a quando a estrutura mudar (os prompts de refatoração fazem isso).
```
CMakeLists.txt, README.md, CHANGELOG.md, LICENSE, CLAUDE.md, .clangd, .gitignore
include/spike/   ast.hpp cli.hpp colors.hpp diagnostics.hpp file_reader.hpp lexer.hpp parser.hpp token.hpp
src/             main.cpp; um módulo por pasta: io/ (file_reader), diagnostics/, cli/
src/lexer/       token.cpp, lexer_core (laço principal, navegação, comentários), lexer_scan (números, textos, nomes), lexer_hints.hpp (interno)
src/parser/      parser_core (navegação), parser_program (cabeçalho, var), parser_stmt (atribuição, escreva, leia), parser_control (se, laços), parser_expr (precedência)
tests/           main.cpp (só chama os run_<grupo>_tests() e imprime o resultado), programs.cmake
tests/support/   test_support.hpp (SPIKE_EXPECT, g_failures, tokens_of) e registry.hpp (run_<grupo>_tests)
tests/<modulo>/  ast, cli, diagnostics, file_reader, lexer, parser, token: um arquivo por assunto
tests/programs/  valid/ e invalid/: programas .por usados como testes
examples/        programas de exemplo; também rodam como testes válidos
```

Alvos do CMake:

- `spike_warnings`: biblioteca INTERFACE com `-Wall -Wextra -Wpedantic` (GCC e Clang).
- `spike_core`: biblioteca estática com tudo, menos o `main`. As fontes ficam em listas por módulo (`SPIKE_LEXER_SOURCES` etc.). `src/` é include PRIVATE: headers internos de módulo ficam em `src/<modulo>/`, nunca em `include/`.
- `spike`: executável (`src/main.cpp`).
- `spike_tests`: executável de testes (`tests/main.cpp` mais os arquivos de `tests/<modulo>/`, listados um a um).

Testes no CTest: `spike_version_check` (roda `spike --version` e espera a versão do `project()`) e `spike_unit_tests` (espera a frase `all tests passed`). Cada `.por` em `examples/` e `tests/programs/valid/` vira um teste que roda `spike tokens` e espera `parse: OK`. Cada `.por` em `tests/programs/invalid/` começa com o código esperado (`e400_missing_entao.por`) e o teste exige `[E400]` na saída. Basta criar o arquivo e reconfigurar. C++17 sem extensões. A macro `SPIKE_VERSION` vem de `PROJECT_VERSION`.

## Convenções de código

- `namespace spike`; guardas de header `SPIKE_<NOME>_HPP`; extensões `.hpp` e `.cpp`.
- Todo nó da AST guarda `line` e `col`.
- A AST usa `std::variant` dentro dos structs envelope `Expr` e `Stmt`, com `std::unique_ptr` (`ExprPtr`, `StmtPtr`, `StmtList`) e os helpers `make_expr` e `make_stmt`. Não há classes virtuais.
- Operadores (`op`) e tipos (`type`) da AST são `std::string` ("+", "e", "inteiro"...).
- Cada fase tem a mesma forma: recebe a entrada, acumula erros num objeto `Diagnostics` e expõe `had_error()` e `diagnostics()`.

API pública:

- `Lexer(source, filename)`: `tokenize()` devolve `vector<Token>` terminando em `END_OF_FILE`.
- `Parser(tokens, filename, source)`: `parse()` devolve `Program`; `parse_expr()` devolve `ExprPtr`.
- `Diagnostics(filename, source)`: `error(code, msg, line, col, hint)`, `warning(...)`, `print_all()`, `has_errors()`, `error_count()`, `warning_count()`.
- `read_file(path)` devolve `ReadResult{ok, content, error}`.
- `parse_args(argc, argv)` devolve `CliArgs`; `print_help()`; `print_version()`.
- `colors.hpp`: `red`, `green`, `yellow`, `cyan`, `gray`, `caramel`, `bold` e `init_colors()`, que liga o modo ANSI no console do Windows.

AST:

- Expressões: `NumberExpr{value, is_integer}`, `StringExpr`, `BoolExpr`, `VarExpr{name}`, `BinaryExpr{op, left, right}`, `UnaryExpr{op, operand}`.
- Comandos: `AssignStmt{var_name, value}`, `EscrevaStmt{args, newline}`, `LeiaStmt{var_names}`, `SeStmt{condition, then_branch, else_branch}`, `EnquantoStmt{condition, body}`, `ParaStmt{var_name, from, to, step, body}`, `RepitaStmt{body, condition}`.
- `VarDecl{name, type, initializer, line, col}` (o parser nunca preenche `initializer`).
- `Program{name, globals, body, line}`.

## A linguagem hoje

```
algoritmo "exemplo"
var
  a, b : inteiro
  nome : caractere
inicio
  a <- 10
  b := a * 2 + 1
  se b > 20 e nao (a = 5) entao
    escreval("grande")
  senao
    escreval("pequeno")
  fimse
  para a de 1 ate 3 passo 1 faca
    escreva(a, " ")
  fimpara
fimalgoritmo
```

Regras léxicas:

- Palavras-chave não diferenciam maiúsculas de minúsculas. `caracter` vale como `caractere`.
- Palavras-chave: algoritmo fimalgoritmo inicio var inteiro real caractere caracter logico se entao senao fimse enquanto faca fimenquanto para de ate passo fimpara repita escreva escreval leia verdadeiro falso e ou nao.
- Identificadores: letra ASCII ou `_`, seguida de letras ASCII, dígitos ou `_`. Letra acentuada num nome dá E001.
- Números: dígitos com parte decimal opcional (`3.14`). `3.` dá E003.
- Textos: entre aspas duplas, na mesma linha. Escapes `\"`, `\\`, `\n`, `\t`. Escape desconhecido fica como barra mais o caractere.
- Comentários: `//` até o fim da linha e `{ ... }` em bloco, que pode ter várias linhas (sem fechar dá E004).
- Operadores e símbolos: `+ - * / = <> < > <= >= <- := ( ) , : .`
- O token `ATE_QUE` existe no enum, mas o lexer não o emite; `repita ... ate condicao` usa `ATE`.

Regras do parser:

- Programa: `algoritmo "nome" [var ...] inicio ... fimalgoritmo`. O nome é um texto entre aspas.
- Seção `var`: linhas `a, b : tipo`, sem valor inicial.
- `escreva`, `escreval` e `leia` exigem parênteses. Atribuição: `nome <- expr` ou `nome := expr`.
- `se ... entao ... [senao ...] fimse`; `enquanto ... faca ... fimenquanto`; `para v de a ate b [passo c] faca ... fimpara`; `repita ... ate condicao`.
- Precedência, do mais fraco para o mais forte: `ou`, `e`, `nao`, comparações (`= <> < > <= >=`), `+ -`, `* /`, menos unário, valores e parênteses.

## Diagnósticos

```
erro[E001]: caractere desconhecido '@'
 --> arquivo.por:5:10
     |
   4 | inicio
   5 |   x <- 5 @
     |          ^
   6 |   y <- 3
     |
     = dica: Portugol não usa este caractere. Verifique se não é um erro de digitação.
```

Faixas de códigos:

- E001-E004: lexer (caractere desconhecido, string não fechada, número malformado, comentário de bloco não fechado).
- E100-E105: estrutura do programa e seção `var`.
- E200-E201: expressões.
- E300-E305: comandos (desconhecido, parênteses de escreva e leia, símbolo de atribuição).
- E400-E401: `se`.
- E500-E506: `enquanto` e `para`.
- E600: `repita`.
- E700-E799: análise semântica (a criar).
- E800-E899: construções novas do parser: escolha, vetores, subprogramas (a criar).
- E900-E999: gerador de C, compilador C e erros internos (a criar).
- W001+: avisos (a criar). R001+: erros em tempo de execução dos programas gerados (a criar).

Todo código novo é registrado em `docs/errors.md` (criado no prompt 029) com mensagem, causa e como corrigir.

Códigos de saída do `spike`: 0 sucesso; 1 erros no programa do usuário; 2 uso errado da linha de comando; 3 erro interno ou compilador C ausente (ainda não usado). Um programa gerado que falha em tempo de execução sai com 101.

## Build e testes

No Windows (MSYS2 UCRT64, GCC em `C:\msys64\ucrt64\bin`), use o gerador Ninja:

```
cmake -S . -B build -G Ninja
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`C:\msys64\ucrt64\bin` precisa vir antes de qualquer outro MinGW no PATH (o Git traz um `/mingw64/bin`). Se a ordem estiver errada, os executáveis carregam a `libstdc++-6.dll` errada e o CTest falha com `0xc0000139`. No Bash: `PATH="/c/msys64/ucrt64/bin:$PATH" ctest --test-dir build --output-on-failure`. No Linux/macOS, os mesmos comandos sem `-G Ninja`.

O executável fica em `build/spike` (`build/spike.exe` no Windows). Os testes usam um executor próprio: a macro `SPIKE_EXPECT(cond)` conta falhas, e o programa imprime `all tests passed` quando não há nenhuma. O CTest procura exatamente essa frase. Nunca mude essa frase.

## Regras de trabalho

Comunicação:

- Responda em português do Brasil, em linguagem simples, passo a passo, sem pular etapas e sem supor que o autor conhece o assunto.
- Ao dar comandos, explique o que cada um faz.
- Ao terminar, diga o que mudou e por quê, em poucas linhas.

Trabalho:

1. Um prompt é uma tarefa e vira um commit. Não misture assuntos. Se a tarefa parecer grande demais para um commit, pare e proponha como dividir.
2. Nunca rode `git add`, `git commit`, `git push`, `git tag`, `git reset` nem apague branches. Só sugira a mensagem de commit, em Conventional Commits, em inglês, com título de até 72 caracteres.
3. Idioma: código, nomes de arquivos, testes, commits e documentação técnica em inglês. Textos que o usuário do Spike vê (erros, dicas, avisos, ajuda da CLI) em português com acentos.
4. Não escreva comentários no código novo. Use nomes claros. Só comente quando o motivo não for óbvio e não couber num nome. Não mexa nos comentários existentes, a não ser quando o trecho for reescrito.
5. README, CHANGELOG e docs: tom humano e direto. Evite palavras vazias (robusto, poderoso, seamless, leverage, comprehensive), emojis e listas inchadas.
6. Nenhum arquivo com mais de 300 linhas, nem de código nem de teste. Divida em pastas por módulo: `src/<modulo>/` e `tests/<modulo>/`. Headers públicos ficam em `include/spike/`.
7. Zero warnings com `-Wall -Wextra -Wpedantic`.
8. Todo comportamento novo tem teste. Todo bug corrigido ganha um teste que falhava antes. Quando possível, escreva o teste primeiro.
9. Antes de editar, leia os arquivos relevantes. Não invente nomes de funções: confira no código.
10. Refatoração não muda comportamento. A suíte inteira passa antes e depois, e o número de testes não diminui.
11. Dúvida de produto (o que o Spike deve fazer): pergunte com AskUserQuestion, mostrando a opção recomendada primeiro. Dúvida técnica pequena: decida e registre na seção "Decisões" da resposta final.
12. Só diga que terminou depois de: build limpa do zero, CTest verde, resumo do que mudou.
13. Portabilidade: o produto precisa funcionar em Windows, Linux e macOS. Não use API só de POSIX ou só de Windows sem `#ifdef` e sem alternativa.
14. Não adicione dependência externa (biblioteca C++, pacote npm etc.) sem perguntar, salvo quando o prompt mandar.
15. Segurança: nunca monte comando de shell juntando texto do usuário. Execute programas passando os argumentos como lista.
16. A versão do produto existe só em CMakeLists.txt (project VERSION). Nunca escreva o número em outro lugar.

## Plano

Os prompts são numerados de 020 a 076; cada um é uma tarefa e um commit.

- A (020-022) Preparação: CLAUDE.md, licença e versão única, exemplos do histórico.
- B (023-028) Base: dividir arquivos grandes, enums na AST, especificação da linguagem.
- C (029-041) Fatia vertical até o primeiro executável (v0.2).
- D (042-058) Linguagem útil: operadores, constantes, vetores, funções, funções prontas, erros de execução, recuperação de erros, avisos, diagnósticos (v0.3).
- E (059-064) Pronto para usar: CI, portabilidade, instalador do compilador C, release, sugestões, documentação (v0.4).
- F (065-069) Ferramentas: extensão do VS Code, formatador, otimizador, fuzzing (v0.5).
- G (070-076) Estável: interpretador, REPL, WebAssembly, servidor de linguagem, random, v1.0.

## Como tratar este arquivo

O CLAUDE.md deve ficar abaixo de 200 linhas. Quando um prompt mudar a estrutura, a linguagem ou as regras, atualize o trecho correspondente no mesmo commit.
