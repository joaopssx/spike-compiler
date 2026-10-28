# Primeiros passos

Este guia ainda está sendo escrito; a versão completa chega junto com a v0.4.

## 1. Instalar o Spike

Por enquanto o Spike é compilado a partir do código: veja "Installing" no [README](../README.md).

## 2. Instalar um compilador C

O Spike transforma o seu `.por` em C e usa um compilador C para gerar o executável. O comando `spike setup` cuida disso.

### Windows

```
spike setup
```

O Spike mostra o que vai baixar (o w64devkit, um GCC portátil de cerca de 64 MB, do GitHub), pergunta se pode continuar e instala em `%LOCALAPPDATA%\Spike`. O arquivo só é usado depois de conferir a impressão digital (SHA-256) gravada no próprio Spike. Daí em diante o Spike encontra esse compilador sozinho; não é preciso mexer no PATH.

- `spike setup --check`: diz se já existe um compilador C (código de saída 0) ou não (3).
- `spike setup --force`: baixa e instala de novo.
- `spike setup --remove`: apaga a pasta do compilador instalado pelo Spike (pede confirmação).
- `--yes`: não pergunta nada (útil em scripts).

### Linux

```
spike setup
```

O Spike não instala nada no Linux: ele detecta a distribuição e mostra o comando certo, como `sudo apt install build-essential` no Debian e no Ubuntu. Se o compilador já existe, ele avisa e termina.

### macOS

```
xcode-select --install
```

Isso instala as ferramentas de linha de comando da Apple, com o `cc` que o Spike usa. `spike setup` mostra a mesma dica.

## 3. O primeiro programa

(em breve)
