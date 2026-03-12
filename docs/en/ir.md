# Intermediate Representation

Spike v1 keeps AST to C generation as the main path.

Spike v2 introduces an IR layer to support:

- optimization passes
- alternative backends
- richer debugging tools
- a cleaner separation between frontend and backend

Current command:

```bash
spike ir examples/basic/hello.por
spike ir examples/basic/hello.por --json
```

The current IR is textual and educational, already good enough for inspection and for the first optimization pass.
