#include "codegen/c_compile_support.hpp"
#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/file_reader.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::generate_from_source;

#ifdef SPIKE_TEST_C_COMPILER

using spike_tests::compile_c;
using spike_tests::read_all;
using spike_tests::run_to_file;

void test_generated_c_compiles_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"compila\"\n"
        "var\n  a, i : inteiro\n  r : real\n  t, u : caractere\n  b, usado : logico\n"
        "inicio\n"
        "  a <- 10\n  i <- a * 2 - -3\n  r <- a / 4 + 0.5\n  r <- -r\n"
        "  t <- \"Olá, \\\"mundo\\\"\\n\"\n  u <- t + \"!\"\n"
        "  b <- (t < u) e nao (a = i) ou (r >= 2)\n  usado <- b = falso\n"
        "  escreval(a, \" \", i, \" \", r, \" \", b, \" \", usado)\n"
        "fimalgoritmo\n",
        "compila.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_compile_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".out") == "10 23 -3 verdadeiro falso\n");
}

void test_hello_example_runs() {
    const auto file = spike::read_file(std::string(SPIKE_SOURCE_DIR) + "/examples/hello.por");
    SPIKE_EXPECT(file.ok);
    const auto gen = generate_from_source(file.content, "hello.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_hello_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    const std::string output = read_all(base + ".out");
    if (output != "Olá, mundo!\n") std::cerr << "  hello printed: " << output << "\n";
    SPIKE_EXPECT(output == "Olá, mundo!\n");
}

void test_procedures_compile_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"procs\"\n"
        "var\n  total, i : inteiro\n"
        "procedimento soma(x: real, vezes: inteiro)\n"
        "var\n  i : inteiro\n  parcial : real\n  marcas : vetor[1..2] de caractere\n"
        "inicio\n"
        "  para i de 1 ate vezes faca\n    parcial <- parcial + x\n  fimpara\n"
        "  marcas[1] <- \"=\"\n"
        "  total <- total + vezes\n  escreval(marcas[1], parcial)\n"
        "fimprocedimento\n"
        "procedimento conta(n: inteiro)\n"
        "inicio\n  se n > 0 entao\n    escreva(n, \" \")\n    conta(n - 1)\n  fimse\n"
        "fimprocedimento\n"
        "inicio\n  i <- 7\n  soma(1.5, 2)\n  soma(i, 1)\n  conta(3)\n  escreval(total, i)\n"
        "fimalgoritmo\n",
        "procs.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_procedures_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".out") == "=3\n=7\n3 2 1 37\n");
}

void test_functions_compile_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"funcs\"\nvar\n  i : inteiro\n"
        "funcao fat(n: inteiro): inteiro\ninicio\n  se n <= 1 entao\n    retorne 1\n  fimse\n"
        "  retorne n * fat(n - 1)\nfimfuncao\n"
        "funcao media(a, b: real): real\ninicio\n  retorne (a + b) / 2\nfimfuncao\n"
        "funcao par(n: inteiro): logico\ninicio\n  retorne n mod 2 = 0\nfimfuncao\n"
        "funcao nome(): caractere\ninicio\n  retorne \"Spike\"\nfimfuncao\n"
        "procedimento mostra(n: inteiro)\ninicio\n  se par(n) entao\n    retorne\n  fimse\n"
        "  escreval(nome(), \" \", n)\nfimprocedimento\n"
        "inicio\n  i <- fat(5)\n  mostra(i)\n  mostra(3)\n"
        "  escreval(media(i, 1), \" \", par(i))\nfimalgoritmo\n",
        "funcs.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_functions_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".out") == "Spike 3\n60.5 verdadeiro\n");
}

void test_references_compile_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"refs\"\nvar\n  x, y : inteiro\n  nomes : vetor[1..2] de caractere\n"
        "procedimento troca(var a, b: caractere)\nvar\n  t : caractere\ninicio\n"
        "  t <- a\n  a <- b\n  b <- t\nfimprocedimento\n"
        "funcao incrementa(var n: inteiro, salto: inteiro): inteiro\ninicio\n"
        "  n <- n + salto\n  retorne n\nfimfuncao\n"
        "inicio\n  nomes[1] <- \"Ana\"\n  nomes[2] <- \"Bia\"\n  troca(nomes[1], nomes[2])\n"
        "  troca(nomes[1], nomes[1])\n  y <- incrementa(x, 5)\n"
        "  escreval(nomes[1], \" \", nomes[2], \" \", x, \" \", y)\nfimalgoritmo\n",
        "refs.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_references_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".out") == "Bia Ana 5 5\n");
}

void test_builtins_compile_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"prontas\"\nvar\n  i : inteiro\n  r : real\n"
        "inicio\n  i <- -7\n  r <- 2.5\n"
        "  escreval(abs(i), \" \", abs(-r), \" \", raiz(16), \" \", pot(2, 10))\n"
        "  escreval(arredonda(r), \" \", trunca(-r), \" \", piso(-r), \" \", teto(r))\n"
        "  escreval(arredonda(seno(0) + cosseno(0) + tangente(0)), \" \", log(100), \" \", "
        "exp(0))\n  i <- aleatorio(3, 3)\n  aleatorio(1, 2)\n  escreval(i)\nfimalgoritmo\n",
        "prontas.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_builtins_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".out") == "7 2.5 4 1024\n3 -2 -3 3\n1 2 1\n3\n");
}

void test_text_builtins_compile_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"textos\"\nvar\n  t : caractere\n"
        "inicio\n  t <- \"ação\"\n"
        "  escreval(tamanho(t), \" \", maiusculo(t), \" \", copia(t, 2, 2), \" \", pos(\"ã\", t))\n"
        "  escreval(concatena(minusculo(\"ÁRVORE\"), paraTexto(2.5)), \" \", paraInteiro(\" 7 \") + 1)\n"
        "  escreval(ehNumero(\"3,5\"), \" \", paraReal(\"3,5\"), \" \", ord(\"ç\"), \" \", chr(65))\n"
        "fimalgoritmo\n",
        "textos.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_text_builtins_test";
    const bool compiled = compile_c(gen.result.c_source, base);
    SPIKE_EXPECT(compiled);
    if (!compiled) return;
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".out") ==
                 "4 AÇÃO çã 3\nárvore2.5 8\nverdadeiro 3.5 231 A\n");
}

#else

void test_text_builtins_compile_without_warnings() {}

void test_builtins_compile_without_warnings() {}

void test_references_compile_without_warnings() {}

void test_functions_compile_without_warnings() {}

void test_generated_c_compiles_without_warnings() {}

void test_procedures_compile_without_warnings() {}

void test_hello_example_runs() {}

#endif

} // namespace

namespace spike_tests {

void run_codegen_compile_tests() {
    test_generated_c_compiles_without_warnings();
    test_hello_example_runs();
    test_procedures_compile_without_warnings();
    test_functions_compile_without_warnings();
    test_references_compile_without_warnings();
    test_builtins_compile_without_warnings();
    test_text_builtins_compile_without_warnings();
}

} // namespace spike_tests
