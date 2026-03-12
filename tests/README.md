# Tests

Spike uses a mix of smoke tests, future unit tests and future integration tests.

## Current local command

```bash
ctest --test-dir build --output-on-failure
```

## Planned layout

- `tests/unit/` for lexer, parser, AST, sema and IR tests
- `tests/integration/` for CLI and end-to-end scenarios
- `tests/snapshots/` for AST, diagnostics, IR and generated C outputs
- `tests/data/` for reusable valid and invalid sample programs
