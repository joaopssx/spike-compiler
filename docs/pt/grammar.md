# Gramatica

O Spike atualmente suporta um MVP enxuto de Portugol.

```text
programa -> "algoritmo" STRING bloco_var "inicio" lista_comandos "fimalgoritmo"
bloco_var -> "var" lista_declaracoes | vazio
comando -> atribuicao | escreva | leia | se | enquanto
expressao -> igualdade
```

Veja `docs/grammar.md` para anotacoes adicionais da gramatica atual.
