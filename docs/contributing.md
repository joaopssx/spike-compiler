# Contributing to Spike

Thank you for considering a contribution.

## Workflow

1. fork the repository
2. create a feature branch
3. implement your change with focused commits
4. run the local build and tests
5. open a pull request with a clear description

## Local Checks

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Guidelines

- keep the code educational and easy to read
- prefer small, explicit abstractions
- preserve the current CLI unless the change improves usability clearly
- update docs and examples when behavior changes
- include tests whenever practical

## Reporting Issues

Please include:

- the Spike command you ran
- the `.por` source that triggers the issue
- the exact output or diagnostic message
- your platform and compiler information
