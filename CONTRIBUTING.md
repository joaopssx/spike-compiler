# Contributing to Spike

Thanks for your interest in contributing.

## Quick start

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Guidelines

- keep changes focused and easy to review
- preserve the compiler-first direction of the project
- update docs and examples with behavior changes
- add tests whenever practical

## Pull requests

Use the pull request template and explain both the change and its motivation.

More detailed contribution guides live in `docs/en/contributing.md` and `docs/pt/contributing.md`.
