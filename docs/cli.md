# Comandos e flags

Tudo o que o `spike` aceita na linha de comando. A mesma lista aparece com `spike --help` (ou só `spike`, sem nada).

## Compilar

```
spike <arquivo.por> [flags]
```

Lê o programa, confere erros, gera o C, chama o compilador C e grava o executável. Sem flags, o executável fica na mesma pasta do `.por`, com o mesmo nome (`media.por` vira `media.exe` no Windows e `media` no Linux e no macOS).

| Flag | O que faz |
| --- | --- |
| `-o <arquivo>` | onde gravar o executável (padrão: o nome do .por, na mesma pasta) |
| `--run` | depois de compilar, executa o programa |
| `--c` | guarda o código C gerado ao lado do executável (`<nome>.c`) |
| `--time` | mostra quanto tempo cada etapa levou, em milissegundos |
| `--verbose` | conta o que o compilador está fazendo em cada etapa |
| `--ast` | mostra a árvore do programa e não compila |
| `--no-warnings` | não mostra os avisos (`aviso[W...]`) |
| `--warnings-as-errors` | qualquer aviso faz a compilação falhar |

As flags podem ser combinadas e vir em qualquer ordem: `spike media.por --c --time -o saida/media`.

- Com `--run`, o Spike sai com o mesmo código de saída do programa.
- `--time` e `--verbose` escrevem na saída de erros (stderr), para não misturar com a saída do programa.
- `--c` grava o C sem as diretivas `#line`, para ficar mais fácil de ler. Veja a [lição 9 do tutorial](tutorial/09-ver-o-codigo-c.md).

## Outros comandos

| Comando | O que faz |
| --- | --- |
| `spike check <arq>` | só confere o programa, sem gerar executável; não precisa de compilador C |
| `spike tokens <arq>` | lista os tokens do arquivo (modo debug) |
| `spike dev <arq>` | roda todas as fases e grava tokens, árvore, símbolos, tipos, C e tempos em `<arq>.dev/` |
| `spike setup` | instala um compilador C (Windows) ou mostra como instalar; `--check`, `--force`, `--remove`, `--yes` |
| `spike fetch` | mostra o ambiente do Spike (cole a saída ao pedir ajuda) |
| `spike --help` | mostra a ajuda |
| `spike --version` | versão do compilador |
| `spike --explain <código>` | explica um código de erro: `spike --explain E400`; sem código, lista todos |

`spike random`, um gerador de programas aleatórios, está planejado para a v1.0. Hoje ele só avisa isso e sai com código 2.

### spike setup

| Opção | O que faz |
| --- | --- |
| (nenhuma) | Windows: mostra o que vai baixar, pede confirmação, baixa o w64devkit, confere o SHA-256 e instala em `%LOCALAPPDATA%\Spike`. Linux e macOS: mostra o comando de instalação do seu sistema. |
| `--check` | só diz se já existe um compilador C: código 0 se existe, 3 se não |
| `--force` | instala de novo, mesmo que já exista um compilador |
| `--remove` | apaga a pasta do compilador instalado pelo Spike (pede confirmação) |
| `--yes` | não faz perguntas (útil em scripts) |

```
spike setup --check
```

```
ok: o compilador C já está instalado: C:\msys64\ucrt64\bin\gcc.exe (gcc)
```

O download só acontece quando você roda `spike setup` e confirma. Nenhum outro comando baixa nada.

### spike fetch

Mostra a versão, o sistema, o compilador C encontrado e se a pasta temporária aceita gravação. Ao pedir ajuda, cole essa saída:

```
spike fetch
```

```
    / \__      Spike 0.4.0
   (    @\___  sistema: Windows (x86_64)
   /         O compilador C: gcc em C:\msys64\ucrt64\bin\gcc.exe
  /   (_____/  versão do C: gcc.exe (Rev5, Built by MSYS2 project) 16.1.0
 /_____/   U   SPIKE_CC: não definido (o Spike procura no PATH)
               pasta temporária: gravável (C:\Users\jpsen\AppData\Local\Temp)

tudo certo: o Spike consegue gerar executáveis.
pedindo ajuda? cole esta saída na issue: https://github.com/joaopssx/spike-compiler/issues
```

Sai com 0 quando está tudo certo e com 3 quando falta algo (por exemplo, nenhum compilador C).

## Cores

| Flag | O que faz |
| --- | --- |
| `--color=auto` | padrão: cores só no terminal; `NO_COLOR` desliga, `FORCE_COLOR` liga |
| `--color=always` | sempre com cores |
| `--color=never` | nunca com cores |

A flag vale com qualquer comando.

## Códigos de saída

| Código | Quando |
| --- | --- |
| 0 | deu tudo certo |
| 1 | o programa tem erros (léxicos, de sintaxe, de semântica), o arquivo não foi encontrado ou o programa usa algo que o gerador ainda não suporta (E901) |
| 2 | a linha de comando está errada (flag ou comando desconhecido, flag sem o valor que ela pede) |
| 3 | erro interno ou do compilador C (E900, E902 a E907) |
| 101 | um programa gerado pelo Spike parou com um erro de execução (`erro de execução[R...]`) |

Com `--run`, o código de saída é o do programa que rodou.

## Variáveis de ambiente

| Variável | Efeito |
| --- | --- |
| `SPIKE_CC` | caminho de um compilador C para o Spike usar, no lugar do que ele encontraria sozinho. Se o caminho não existir, é o erro E902. |
| `SPIKE_SEED` | um número: os programas gerados usam como semente de `aleatorio`, e repetem a mesma sequência em qualquer sistema. Sem ela, a semente vem do relógio. |
| `NO_COLOR` | se tiver qualquer valor, desliga as cores do Spike e dos erros de execução dos programas gerados. |
| `FORCE_COLOR` | se tiver um valor diferente de `0`, liga as cores do Spike mesmo fora de um terminal. `NO_COLOR` e `--color=never` ganham dela. |

Sem `SPIKE_CC`, o Spike procura o compilador C nesta ordem: o instalado pelo `spike setup`, a pasta embutida ao lado do `spike`, e o `PATH` (`gcc`, `clang`, `cc`).
