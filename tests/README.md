# Tests

Spike currently ships with lightweight smoke tests through CTest.

Run them locally with:

```bash
ctest --test-dir build --output-on-failure
```

Planned additions:

- golden tests for token streams
- golden tests for AST dumps
- semantic error regression tests
- generated C snapshots
