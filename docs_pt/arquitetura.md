# Arquitetura

O Spike foi montado com uma estrutura pequena e direta, pensando em estudo e evolucao gradual.

## Fluxo atual

Nesta etapa o fluxo e:

1. A CLI recebe `spike tokens arquivo.por`.
2. O modulo de leitura carrega todo o arquivo para uma `std::string`.
3. O `Lexer` percorre o texto da esquerda para a direita e pode guardar o nome do arquivo para melhorar os diagnosticos.
4. Cada trecho reconhecido vira um `Token`.
5. A CLI imprime o nome dos tokens ou uma visao detalhada com lexema, linha e coluna.

## Separacao de responsabilidades

### `TokenType`

`TokenType` define quais categorias de token existem. Ele e um `enum class`, o que deixa o codigo mais seguro e evita misturar tipos de token com inteiros soltos.

### `Token`

`Token` guarda:

- o tipo do token
- o lexema original
- linha
- coluna

A CLI pode imprimir so o nome do token ou uma saida detalhada com lexema, linha e coluna.

### `Lexer`

O `Lexer` recebe o texto completo e produz `std::vector<Token>`.

Ele foi mantido simples:

- um indice para a posicao atual
- contadores de linha e coluna
- funcoes pequenas para cada tipo de leitura
- uma passada unica que ignora espacos e comentarios `//` antes de ler o proximo token
- validacao de um conjunto pequeno de escapes em strings

Essa abordagem ajuda a estudar o processo sem esconder a logica em muitas camadas.

### `FileReader`

O modulo de leitura de arquivo foi separado do lexer para nao misturar I/O com analise lexica.

### `main.cpp`

`main.cpp` cuida apenas da interface de linha de comando:

- validar argumentos
- ler arquivo
- chamar o lexer
- imprimir os tokens
- alternar entre a saida normal e a saida `--verbose`

### `tests/`

O projeto agora inclui um executavel pequeno de testes automatizados do lexer. Os testes cobrem o comportamento central desta etapa:

- reconhecimento de tokens
- ignorar comentarios
- tratamento de escapes em strings
- mensagens de erro
- leitura de fixtures a partir de arquivos reais

## Decisoes de design

1. O projeto usa poucos arquivos, mas cada um tem um papel claro.
2. O lexer trabalha em uma passada unica, que e o jeito mais facil de entender e depurar nessa fase.
3. O token guarda o lexema original porque isso sera util no parser e em mensagens de erro.
4. Existe um token interno de fim de arquivo, mas ele nao e exibido no comando `tokens` para manter a saida limpa.
5. Os testes automatizados entraram agora para deixar as proximas mudancas no lexer mais seguras sem adicionar um framework pesado.
6. O suporte a escapes foi mantido pequeno de proposito, cobrindo apenas o necessario nesta fase.
