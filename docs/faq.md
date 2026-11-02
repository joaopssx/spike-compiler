# Perguntas frequentes

## "O compilador C não foi encontrado"

O Spike traduz o seu programa para C e precisa de um compilador C para criar o executável. Sem ele, aparece o erro E902. Resolva com:

```
spike setup
```

No Windows, esse comando baixa e instala um compilador (o w64devkit, cerca de 64 MB), depois de pedir confirmação. No Linux, ele mostra o comando da sua distribuição, como `sudo apt install build-essential`. No macOS, o comando é `xcode-select --install`.

Depois, `spike fetch` deve terminar com `tudo certo: o Spike consegue gerar executáveis.`. Se você definiu a variável `SPIKE_CC`, confira se o caminho está certo ou apague a variável. Enquanto isso, `spike check programa.por` confere o programa sem precisar do compilador C.

## Os acentos saem errados no Windows

O Spike e os programas que ele gera trocam o console do Windows para UTF-8 quando começam e devolvem a configuração antiga quando terminam. Então `escreval("ação")` aparece certo no Prompt de Comando, no PowerShell e no Terminal do Windows.

Se mesmo assim aparecer `aÃ§Ã£o` ou `a??o`, confira três coisas:

1. **O arquivo `.por` está em UTF-8?** Se ele foi salvo como "ANSI" (Windows-1252), o Spike converte sozinho e mostra o aviso W010 com o passo a passo para salvar em UTF-8. Salve assim e o aviso some.
2. **A fonte do console tem as letras?** Fontes antigas, como "Fontes de varredura", não têm todas as letras. Use Consolas, Cascadia Mono ou a fonte padrão do Terminal do Windows.
3. **A saída foi redirecionada para um arquivo?** `programa.exe > saida.txt` grava em UTF-8. Abra o arquivo num editor que entenda UTF-8 (Bloco de Notas, VS Code).

## O antivírus bloqueou o executável gerado

Isso é quase sempre um falso positivo. Antivírus desconfiam de executáveis novos, sem assinatura digital e que ninguém mais tem no computador, e todo programa que você compila é exatamente assim. Programas feitos com o GCC do MinGW, que o Spike usa no Windows, costumam cair nessa regra.

O que fazer:

1. Confira que o executável é mesmo o seu: compile de novo o seu `.por` e veja se o aviso volta.
2. No Windows Defender, abra "Proteção contra vírus e ameaças", "Histórico de proteção", escolha o item e "Permitir no dispositivo".
3. Para não repetir, adicione a pasta onde você guarda os exercícios em "Exclusões". Exclua só essa pasta, nunca o disco inteiro.
4. Se quiser ajudar, envie o arquivo para análise como falso positivo no site do fabricante do antivírus (a Microsoft tem um formulário para isso).

O Spike não baixa nada ao compilar. A única coisa que ele baixa é o compilador C, no `spike setup`, e só depois de você confirmar; o arquivo é conferido por SHA-256 antes de ser usado.

## O macOS diz que o programa "não pode ser aberto"

Isso é o Gatekeeper do macOS. Ele bloqueia programas baixados da internet que não foram assinados pela Apple, como o `spike` dos arquivos de release. Para liberar uma vez: clique com o botão direito no arquivo, escolha "Abrir" e confirme. Pelo Terminal, `xattr -d com.apple.quarantine ./spike` faz o mesmo. Os programas que o próprio Spike gera na sua máquina não são baixados, então não passam por isso.

## O macOS diz que não há compilador C

O Spike usa o `cc` das ferramentas de linha de comando da Apple. Rode `xcode-select --install` no Terminal e confirme a instalação; não é preciso instalar o Xcode inteiro. O erro E902 mostra essa mesma dica.

## Meu programa travou

Quase sempre é um laço que nunca termina: um `enquanto` cuja condição nunca fica falsa, ou um `repita` cujo `ate` nunca fica verdadeiro. Aperte **Ctrl+C** no terminal para parar o programa.

Depois, procure no laço a variável da condição e confira se ela muda lá dentro:

```portugol
algoritmo "conta"
var
  i : inteiro
inicio
  i <- 1
  enquanto i <= 5 faca
    escreval(i)
    i <- i + 1
  fimenquanto
fimalgoritmo
```

Sem a linha `i <- i + 1`, `i` vale 1 para sempre e o laço não para. O programa também pode estar só esperando você digitar algo num `leia`: escreva uma pergunta com `escreva` antes de cada `leia` para saber quando isso acontece.

## Posso usar com o VS Code?

Pode. Escreva o programa no VS Code, salve com a extensão `.por` e use o terminal integrado (menu Terminal, Novo Terminal) para rodar `spike programa.por --run`. O VS Code salva em UTF-8 por padrão, então os acentos funcionam. Ainda não existe uma extensão do Spike com cores e erros no editor; ela está planejada para a v0.5.

## Por que o resultado de 7 / 2 é 3.5?

Porque no Spike `/` é a divisão de verdade e sempre dá um `real`, como na matemática:

```portugol-run
algoritmo "divisoes"
inicio
  escreval(7 / 2)
  escreval(7 div 2)
  escreval(7 mod 2)
fimalgoritmo
```

```output
3.5
3
1
```

Para a divisão inteira, use `div`; para o resto, `mod`. Como `7 / 2` é `real`, ele não pode ser guardado num `inteiro`: escreva `x <- 7 div 2`, ou escolha o arredondamento com `trunca(7 / 2)` ou `arredonda(7 / 2)`.

## Meu arquivo foi salvo pelo Bloco de Notas e o Spike reclamou

Arquivos com BOM (o marcador que o Bloco de Notas põe no começo de arquivos UTF-8) e com fim de linha do Windows (`\r\n`) são lidos normalmente; nenhum dos dois muda linha ou coluna dos erros. Arquivos em "ANSI" são convertidos com o aviso W010. Só um arquivo que não é texto, ou que mistura duas codificações, dá o erro E005.

## Pasta com espaço ou acento funciona?

Funciona. `spike "C:\Users\João\Aulas de Lógica\média.por"` compila e roda, inclusive com `-o "saída com espaço"`. No Windows, o compilador C recebe só nomes simples numa pasta temporária, e o Spike move o executável para o lugar certo no fim, porque o GCC do MinGW não entende caminhos fora da página de código do Windows.
