# Estrutura do projeto

```text
Spike/
|-- CMakeLists.txt
|-- README.md
|-- CHANGELOG.md
|-- docs/
|-- docs_pt/
|-- examples/
|   `-- comentarios_e_strings.por
|-- include/
|   `-- spike/
|       |-- file_reader.h
|       |-- lexer.h
|       `-- token.h
|-- readme_pt.md
|-- tests/
|   |-- fixtures/
|   |   |-- invalid/
|   |   `-- valid/
|   `-- lexer_tests.cpp
`-- src/
    |-- file_reader.cpp
    |-- lexer.cpp
    |-- main.cpp
    `-- token.cpp
```

## Caminhos principais

### `include/spike/`

Cabecalhos publicos do projeto.

### `src/`

Implementacoes em C++.

### `examples/`

Arquivos simples para testar a ferramenta.

### `docs/`

Documentacao do projeto em ingles.

### `docs_pt/`

Documentacao do projeto em portugues.

### `readme_pt.md`

Versao em portugues do README principal.

### `tests/`

Testes automatizados do lexer.

### `tests/fixtures/`

Arquivos de entrada usados pelos testes automatizados.
