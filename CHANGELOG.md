# Changelog

## 0.1.1 - 2026-04-01

### Added

- Support for `//` line comments in the lexer.
- Optional `--verbose` output for the `spike tokens` command.
- Automated lexer tests integrated with `CTest`.

### Changed

- Improved lexical error messages with source line and caret indicator.

## 0.1.0 - 2026-03-31

### Added

- Initial project structure with `CMake`.
- `spike` executable with the `tokens` command.
- Simple file reading system.
- Definitions for `Token`, `TokenType`, and `Lexer`.
- Initial lexer for keywords, operators, symbols, identifiers, numbers, and strings.
- Project documentation in `docs/`.
