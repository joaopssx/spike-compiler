# Primeiros passos

Este guia leva você do download ao primeiro programa rodando, e mostra como ler uma mensagem de erro do Spike.

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
3. Abra um terminal novo (PowerShell ou Prompt de Comando) e confira com `spike --version`.

### Linux e macOS

```
tar -xzf spike-<versão>-linux-x86_64.tar.gz
sudo cp spike-<versão>-linux-x86_64/spike /usr/local/bin/
spike --version
```

O primeiro comando abre o pacote; o segundo copia o `spike` para uma pasta que o terminal já conhece. Troque o nome do arquivo pelo do seu sistema. No macOS, se aparecer "não pode ser aberto", veja [a FAQ](faq.md).

### Conferir o download (opcional)

A página de cada versão tem o `SHA256SUMS.txt`, com a "impressão digital" de cada pacote. Calcule a do arquivo que você baixou e compare:

- Windows (PowerShell): `Get-FileHash spike-<versão>-windows-x86_64.zip`
- Linux: `sha256sum spike-<versão>-linux-x86_64.tar.gz`
- macOS: `shasum -a 256 spike-<versão>-macos-arm64.tar.gz`

Se for diferente, apague o arquivo e baixe de novo.

## 2. Instalar um compilador C com spike setup

O Spike transforma o seu `.por` em C e usa um compilador C para gerar o executável. O comando `spike setup` cuida disso. Rode uma vez, logo depois de instalar:

```
spike setup
```

- **Windows:** o Spike mostra o que vai baixar (o w64devkit, um GCC portátil de cerca de 64 MB, do GitHub), pergunta se pode continuar e instala em `%LOCALAPPDATA%\Spike`. O arquivo só é usado depois de conferir a impressão digital (SHA-256) gravada no próprio Spike. Daí em diante o Spike encontra esse compilador sozinho, sem mexer no PATH.
- **Linux:** o Spike não instala nada. Ele detecta a distribuição e mostra o comando certo, como `sudo apt install build-essential` no Debian e no Ubuntu.
- **macOS:** o Spike mostra o comando `xcode-select --install`, que instala as ferramentas de linha de comando da Apple (não é preciso instalar o Xcode inteiro).

Outras opções: `spike setup --check` só diz se já existe um compilador C; `--force` instala de novo; `--remove` apaga o compilador que o Spike instalou; `--yes` não faz perguntas. Mais detalhes em [cli.md](cli.md).

## 3. O primeiro programa

Abra um editor de texto (o Bloco de Notas serve; o VS Code é melhor) e salve este programa como `ola.por`:

```portugol-run
algoritmo "ola"
inicio
  escreval("Olá, mundo!")
fimalgoritmo
```

```output
Olá, mundo!
```

No terminal, entre na pasta do arquivo com `cd` e compile:

```
spike ola.por
```

```
ok: gerado 'ola.exe' (compilado em 1.4s)
```

O Spike criou `ola.exe` na mesma pasta (no Linux e no macOS, só `ola`). Rode:

```
.\ola.exe
```

```
Olá, mundo!
```

No Linux e no macOS o comando é `./ola`. O executável funciona sozinho: você pode mandar para alguém que não tem o Spike.

Para compilar e rodar de uma vez, use `spike ola.por --run`.

## 4. Quando dá erro

Este programa calcula uma média, mas tem um nome digitado errado na última linha:

```portugol-error E700
algoritmo "media"
var
  nota1, nota2 : real
inicio
  nota1 <- 7
  nota2 <- 8.5
  escreval("Média: ", (nota1 + nota2) / 2)
  escreval(notaa)
fimalgoritmo
```

Ao compilar, o Spike não gera o executável e mostra:

```
erro[E700]: variável 'notaa' não foi declarada
 --> media.por:8:12
     |
   7 |   escreval("Média: ", (nota1 + nota2) / 2)
   8 |   escreval(notaa)
     |            ^
   9 | fimalgoritmo
     |
     = dica: você quis dizer 'nota1'? Declare 'notaa' na seção var.

falhou: 1 erro
Para entender um erro: spike --explain E700
```

Linha por linha:

1. `erro[E700]: variável 'notaa' não foi declarada` diz o que aconteceu. `E700` é o código desse tipo de erro.
2. `--> media.por:8:12` diz onde: arquivo `media.por`, linha 8, coluna 12.
3. As linhas numeradas mostram o seu código em volta do problema: a linha 8 e as vizinhas.
4. A seta `^` aponta o lugar exato, aqui o começo de `notaa`.
5. `dica:` sugere a correção. O Spike percebeu que `notaa` parece com `nota1`.
6. `falhou: 1 erro` resume quantos erros foram encontrados.
7. A última linha lembra que `spike --explain E700` explica esse erro com mais calma, com um exemplo.

Corrija o nome, compile de novo e pronto. Se aparecer mais de um erro, comece pelo primeiro: muitas vezes os outros são consequência dele.

Avisos (`aviso[W...]`) seguem o mesmo formato, mas não impedem a compilação. Erros de execução (`erro de execução[R...]`) aparecem com o programa rodando, por exemplo numa divisão por zero; o programa para e mostra a linha do `.por` onde aconteceu.

## Próximos passos

- [Tutorial](tutorial/README.md): lições curtas, do primeiro programa a funções e vetores.
- [Comandos e flags](cli.md): tudo o que o `spike` aceita.
- [Perguntas frequentes](faq.md): compilador não encontrado, acentos no Windows, antivírus.
- [Catálogo de erros](errors.md): cada código, com causa e correção.
