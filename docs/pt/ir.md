# Intermediate Representation

No ciclo `v1`, o Spike ainda usa AST para gerar C.

Na `v2`, a camada de IR entra para permitir:

- passes de otimizacao
- backends alternativos
- ferramentas de debug mais ricas
- separacao mais limpa entre frontend e backend

Comando atual:

```bash
spike ir examples/basic/hello.por
spike ir examples/basic/hello.por --json
```

A IR atual e textual e educativa, ja suficiente para inspecao e para o primeiro passe de otimizacao.
