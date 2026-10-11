# Changelog

All notable changes to Spike are recorded here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.6]

This is the first version with a changelog. It covers what the rewrite has built so far. Spike reads Portugol programs but does not generate executables yet.

### Added

- Lexer for the current language: case-insensitive keywords, identifiers, integer and decimal numbers, strings with `\"`, `\\`, `\n` and `\t` escapes, `//` line comments and `{ }` block comments.
- Parser that builds an AST for the program header, the `var` section, assignments (`<-` and `:=`), `escreva`, `escreval`, `leia`, `se`/`senao`, `enquanto`, `para` with optional `passo`, and `repita ... ate`.
- Expressions with arithmetic, comparison and logical operators (`e`, `ou`, `nao`), unary minus and parentheses.
- Error messages in Portuguese that show the source line, a caret under the problem and a hint. Lexer errors use E001-E004 and parser errors use E100-E600.
- Command line: `spike --help`, `spike --version` and `spike tokens arquivo.por`, which lists the tokens and checks whether the program parses. `spike arquivo.por` only runs the lexer for now.
- MIT license.
- The version number now lives only in `CMakeLists.txt`.
