# Primeiros passos

Este guia ainda está sendo escrito; a versão completa chega junto com a v0.4.

## 1. Baixar o Spike

Os pacotes prontos ficam na [página de Releases](https://github.com/joaopssx/spike-compiler/releases). Escolha o do seu sistema:

- Windows: `spike-<versão>-windows-x86_64.zip`
- Linux: `spike-<versão>-linux-x86_64.tar.gz` (Ubuntu 22.04, Debian 12 ou mais novos)
- Mac com chip Apple (M1 em diante): `spike-<versão>-macos-arm64.tar.gz`
- Mac com processador Intel: `spike-<versão>-macos-x86_64.tar.gz`

Cada pacote tem o `spike`, a licença, o README, o CHANGELOG e a pasta `examples/`.

### Windows

1. Baixe o `.zip`, clique com o botão direito, escolha "Extrair tudo" e mova a pasta para um lugar fixo, como `C:\Spike`.
2. Coloque essa pasta no `PATH`: menu Iniciar, "Editar as variáveis de ambiente da sua conta", `Path`, Novo.
3. Abra um terminal novo e rode `spike setup` uma vez. **Sem isso o Spike não gera executáveis no Windows**: ele baixa o compilador C (cerca de 64 MB) e confere o arquivo antes de usar.

### Linux e macOS

```
tar -xzf spike-<versão>-linux-x86_64.tar.gz
sudo cp spike-<versão>-linux-x86_64/spike /usr/local/bin/
spike setup
```

Troque o nome do arquivo pelo do seu sistema. No macOS, se aparecer "não pode ser aberto", veja [a FAQ](faq.md).

### Conferir o download

A página de cada versão tem o `SHA256SUMS.txt`, com a "impressão digital" de cada pacote. Calcule a do arquivo que você baixou e compare:

- Windows (PowerShell): `Get-FileHash spike-<versão>-windows-x86_64.zip`
- Linux: `sha256sum spike-<versão>-linux-x86_64.tar.gz`
- macOS: `shasum -a 256 spike-<versão>-macos-arm64.tar.gz`

Se for diferente, apague o arquivo e baixe de novo.

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
