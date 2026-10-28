# Perguntas frequentes

## Os acentos saem errados no Windows

O Spike e os programas que ele gera trocam o console do Windows para UTF-8 quando começam e devolvem a configuração antiga quando terminam. Então `escreval("ação")` aparece certo no Prompt de Comando, no PowerShell e no Terminal do Windows.

Se mesmo assim aparecer `aÃ§Ã£o` ou `a??o`, confira três coisas:

1. **O arquivo `.por` está em UTF-8?** Se ele foi salvo como "ANSI" (Windows-1252), o Spike converte sozinho e mostra o aviso W010 com o passo a passo para salvar em UTF-8. Salve assim e o aviso some.
2. **A fonte do console tem as letras?** Fontes antigas, como "Fontes de varredura", não têm todas as letras. Use Consolas, Cascadia Mono ou a fonte padrão do Terminal do Windows.
3. **A saída foi redirecionada para um arquivo?** `programa.exe > saida.txt` grava em UTF-8. Abra o arquivo num editor que entenda UTF-8 (Bloco de Notas, VS Code).

## Meu arquivo foi salvo pelo Bloco de Notas e o Spike reclamou

Arquivos com BOM (o marcador que o Bloco de Notas põe no começo de arquivos UTF-8) e com fim de linha do Windows (`\r\n`) são lidos normalmente; nenhum dos dois muda linha ou coluna dos erros. Arquivos em "ANSI" são convertidos com o aviso W010. Só um arquivo que não é texto, ou que mistura duas codificações, dá o erro E005.

## Pasta com espaço ou acento funciona?

Funciona. `spike "C:\Users\João\Aulas de Lógica\média.por"` compila e roda, inclusive com `-o "saída com espaço"`. No Windows, o compilador C recebe só nomes simples numa pasta temporária, e o Spike move o executável para o lugar certo no fim, porque o GCC do MinGW não entende caminhos fora da página de código do Windows.

## O macOS diz que o programa "não pode ser aberto"

Isso é o Gatekeeper do macOS. Ele bloqueia programas baixados da internet que não foram assinados pela Apple, como o `spike` dos arquivos de release. Para liberar uma vez: clique com o botão direito no arquivo, escolha "Abrir" e confirme. Pelo Terminal, `xattr -d com.apple.quarantine ./spike` faz o mesmo. Os programas que o próprio Spike gera na sua máquina não são baixados, então não passam por isso.

## O macOS diz que não há compilador C

O Spike usa o `cc` das ferramentas de linha de comando da Apple. Rode `xcode-select --install` no Terminal e confirme a instalação; não é preciso instalar o Xcode inteiro. O erro E902 mostra essa mesma dica.
