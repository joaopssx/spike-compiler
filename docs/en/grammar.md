# Grammar

Spike currently supports a focused Portugol MVP.

```text
programa -> "algoritmo" STRING bloco_var "inicio" lista_comandos "fimalgoritmo"
bloco_var -> "var" lista_declaracoes | vazio
comando -> atribuicao | escreva | leia | se | enquanto
expressao -> igualdade
```

See the legacy grammar notes in `docs/grammar.md` for extra details.
