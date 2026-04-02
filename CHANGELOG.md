# Changelog

## 0.1.2 - 2026-04-01

### Added

- Support for simple string escape sequences: `\"`, `\\`, `\n`, and `\t`.
- Global CLI commands `--help` and `--version`.
- Fixture-based lexer tests under `tests/fixtures/`.
- Additional example source file with comments and escaped strings.

### Changed

- Lexical errors now include the source file name when available.
- The CLI now prints self-contained error messages directly.

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
