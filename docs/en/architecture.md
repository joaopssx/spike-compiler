# Architecture

Spike is split into compiler stages and core services.

- `core/`: source management, diagnostics, session and profiling
- `lexer/`: tokenization
- `parser/`: recursive descent parser
- `ast/`: in-memory syntax tree plus printers
- `sema/`: symbol and type checks
- `backend/`: C emission and native build integration

The long-term design includes IR and optimization passes.
