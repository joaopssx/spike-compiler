# Project structure

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

## Main paths

### `include/spike/`

Public project headers.

### `src/`

C++ implementations.

### `examples/`

Simple files used to test the tool.

### `docs/`

English project documentation.

### `docs_pt/`

Portuguese project documentation.

### `readme_pt.md`

Portuguese version of the main README.

### `tests/`

Automated lexer tests.

### `tests/fixtures/`

Input files used by automated tests.
