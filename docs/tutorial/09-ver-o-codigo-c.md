# 9. Ver o código C

Por dentro, o Spike traduz o seu Portugol para a linguagem C e chama um compilador C para criar o executável. Com a flag `--c`, ele guarda esse arquivo para você ler:

```portugol-run
algoritmo "dobro"
var
  n : inteiro
inicio
  n <- 21
  escreval("dobro: ", n * 2)
fimalgoritmo
```

```output
dobro: 42
```

```
spike dobro.por --c
```

```
ok: gerado 'dobro.exe' (compilado em 0.4s)
ok: código C em 'dobro.c'
```

O `dobro.c` começa com um comentário dizendo de onde veio e depois traz as funções de apoio do Spike (o "runtime"), que cuidam de escrever, ler e conferir erros. O seu programa fica no final do arquivo:

```c
spike_int v_n = 0LL;

int main(void) {
    spike_set_source(spike_source_name, spike_source_lines, 7);
    spike_init();
    v_n = 21LL;
    spike_write_str("dobro: ");
    spike_write_int(spike_mul_int(v_n, 2LL));
    spike_write_newline();
    spike_flush();
    spike_cleanup();
    return 0;
}
```

Comparando com o Portugol:

- `n : inteiro` virou `spike_int v_n = 0LL;`. Em C a variável também tem um tipo; `spike_int` é um inteiro de 64 bits. O `v_` na frente evita confusão com nomes do C, e `0LL` é o zero do tipo `long long`.
- O `inicio` virou `int main(void) {`. Em C, todo programa começa pela função `main`.
- `n <- 21` virou `v_n = 21LL;`. Em C, `=` é atribuição, e cada instrução termina com `;`.
- `escreval("dobro: ", n * 2)` virou uma chamada para cada valor e uma para pular a linha.
- `n * 2` virou `spike_mul_int(v_n, 2LL)`: em vez do `*` direto, o Spike usa uma função. Em C, uma conta de inteiros que passa do maior valor tem comportamento indefinido (pode acontecer qualquer coisa); a função garante que o resultado apenas "dá a volta", sempre do mesmo jeito.
- `spike_init`, `spike_flush` e `spike_cleanup` preparam e encerram o programa (console em UTF-8, saída gravada na tela, memória liberada).

## Laços e decisões

Os comandos de controle ficam parecidos com os do C:

```portugol-run
algoritmo "para"
var
  i, soma : inteiro
  media : real
inicio
  para i de 1 ate 10 faca
    soma <- soma + i
  fimpara
  media <- soma / 10
  se media > 5 entao
    escreval(media)
  fimse
fimalgoritmo
```

```output
5.5
```

```c
int main(void) {
    spike_set_source(spike_source_name, spike_source_lines, 13);
    spike_init();
    v_i = 1LL;
    while (v_i <= 10LL) {
        v_soma = spike_add_int(v_soma, v_i);
        if (!spike_for_advance(&v_i, 1LL)) break;
    }
    v_media = spike_div_real(((spike_real)v_soma), ((spike_real)10LL), 9);
    if (v_media > ((spike_real)5LL)) {
        spike_write_real(v_media);
        spike_write_newline();
    }
    spike_flush();
    spike_cleanup();
    return 0;
}
```

- O `se` virou `if (...) { ... }`. Em C, as chaves `{ }` fazem o papel do `fimse`.
- O `para` virou um `while` com a condição `v_i <= 10LL`. `spike_for_advance` soma o passo e para o laço se a conta passar do maior inteiro.
- `soma / 10` virou `spike_div_real(...)`. `(spike_real)` converte o inteiro em real, porque `/` sempre dá real, e o `9` é a linha do `.por`: se o divisor for zero, o erro de execução mostra essa linha.

Esse é um bom jeito de dar os primeiros passos em C: escreva algo em Portugol, rode com `--c` e compare. O C gerado é feito para ser lido, com uma instrução por linha.

## Exercício

Gere o C do programa abaixo e encontre como o `enquanto` e o `mod` aparecem.

```portugol-run
algoritmo "pares"
var
  i : inteiro
inicio
  i <- 1
  enquanto i <= 6 faca
    se i mod 2 = 0 entao
      escreval(i)
    fimse
    i <- i + 1
  fimenquanto
fimalgoritmo
```

```output
2
4
6
```

## Solução

```
spike pares.por --c
```

```c
    v_i = 1LL;
    while (v_i <= 6LL) {
        if (spike_mod_int(v_i, 2LL, 7) == 0LL) {
            spike_write_int(v_i);
            spike_write_newline();
        }
        v_i = spike_add_int(v_i, 1LL);
    }
```

O `enquanto` virou um `while` e o `mod` virou `spike_mod_int`, que recebe a linha 7 para poder avisar se o divisor for zero.
