# Contribuindo com o Spike

Obrigado por considerar uma contribuicao.

## Fluxo

1. faca um fork do repositorio
2. crie uma branch para a sua feature
3. implemente a mudanca com commits focados
4. rode o build e os testes localmente
5. abra um pull request com uma descricao clara

## Checagens Locais

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Diretrizes

- mantenha o codigo educativo e facil de ler
- prefira abstracoes pequenas e explicitas
- preserve a CLI atual, a menos que a mudanca melhore claramente a usabilidade
- atualize docs e exemplos quando o comportamento mudar
- inclua testes sempre que for pratico

## Reportando Problemas

Inclua:

- o comando do Spike que voce executou
- o fonte `.por` que dispara o problema
- a saida ou diagnostico exato
- informacoes sobre sua plataforma e compilador
