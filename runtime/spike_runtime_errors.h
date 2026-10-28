#ifndef SPIKE_RUNTIME_ERRORS_H
#define SPIKE_RUNTIME_ERRORS_H

#ifdef _WIN32
#include <io.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#endif

#define SPIKE_SHOWN_CALLS 8

static const char* spike_source_file = NULL;
static const char* const* spike_source_text = NULL;
static int spike_source_count = 0;

static inline void spike_set_source(const char* name, const char* const* lines, int count) {
    spike_source_file = name;
    spike_source_text = lines;
    spike_source_count = count;
}

static inline int spike_color_wanted(const char* no_color, int is_terminal) {
    return is_terminal && (no_color == NULL || no_color[0] == '\0');
}

static inline int spike_stderr_color(void) {
#ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0;
    if (!spike_color_wanted(getenv("NO_COLOR"), _isatty(2))) return 0;
    if (handle == INVALID_HANDLE_VALUE || !GetConsoleMode(handle, &mode)) return 0;
    if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) return 1;
    return SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    return spike_color_wanted(getenv("NO_COLOR"), isatty(2));
#endif
}

static inline const char* spike_hint(int code) {
    switch (code) {
    case SPIKE_R_DIV_ZERO:
        return "verifique se o divisor pode valer 0 antes de dividir (use um 'se')";
    case SPIKE_R_INDEX:
        return "o índice precisa estar entre os limites declarados; confira o laço ou a conta "
               "que calcula o índice";
    case SPIKE_R_BAD_INPUT:
        return "digite um valor do tipo que o 'leia' espera; em números reais vale ponto ou "
               "vírgula";
    case SPIKE_R_CONVERSION:
        return "confira se o número cabe em um inteiro ou se o texto tem só um número (use "
               "ehNumero antes de converter)";
    case SPIKE_R_MATH_DOMAIN:
        return "confira o valor antes da conta: raiz só de número sem sinal de menos, log só de "
               "número positivo, potência sem resultado infinito";
    case SPIKE_R_ZERO_STEP:
        return "use um passo positivo para contar para cima ou negativo para contar para baixo";
    case SPIKE_R_END_OF_INPUT:
        return "o programa pediu mais valores do que a entrada tinha; confira quantas vezes o "
               "'leia' é executado";
    case SPIKE_R_OUT_OF_MEMORY:
        return "toda recursão precisa de uma condição de parada que seja alcançada; confira o "
               "'se' que encerra as chamadas";
    case SPIKE_R_MISSING_RETURN:
        return "faça todos os caminhos da função chegarem a um 'retorne' com valor";
    case SPIKE_R_RANDOM_RANGE:
        return "chame aleatorio(menor, maior), com o primeiro número menor ou igual ao segundo";
    case SPIKE_R_TEXT_RANGE:
        return "posições em texto vão de 1 até tamanho(texto); em chr use um código de 1 a "
               "1114111";
    default:
        return NULL;
    }
}

static inline void spike_print_calls(void) {
    int depth = spike_call_depth;
    int shown = 0;
    while (depth > 0 && shown < SPIKE_SHOWN_CALLS) {
        const spike_call* call = &spike_calls[(depth - 1) % SPIKE_SAVED_CALLS];
        if (call->depth != depth) break;
        fprintf(stderr, "  chamado de '%s' na linha %d\n", call->name, call->line);
        --depth;
        ++shown;
    }
    if (depth > 0) fprintf(stderr, "  ... e mais %d\n", depth);
}

static inline void spike_print_error(int code, int line, const char* message) {
    const int color = spike_stderr_color();
    const char* red = color ? "\033[1;31m" : "";
    const char* cyan = color ? "\033[36m" : "";
    const char* gray = color ? "\033[38;5;240m" : "";
    const char* green = color ? "\033[32m" : "";
    const char* reset = color ? "\033[0m" : "";
    const char* hint = spike_hint(code);
    fprintf(stderr, "%serro de execução[R%03d]:%s %s\n", red, code, reset, message);
    if (spike_source_file == NULL) {
        if (line > 0) fprintf(stderr, "  --> linha %d\n", line);
    } else if (line > 0) {
        fprintf(stderr, "%s --> %s:%d%s\n", cyan, spike_source_file, line, reset);
    } else {
        fprintf(stderr, "%s --> %s%s\n", cyan, spike_source_file, reset);
    }
    if (spike_source_text != NULL && line >= 1 && line <= spike_source_count) {
        fprintf(stderr, "     %s|%s\n", gray, reset);
        fprintf(stderr, "%s%4d |%s %s\n", gray, line, reset, spike_source_text[line - 1]);
        fprintf(stderr, "     %s|%s\n", gray, reset);
    }
    spike_print_calls();
    if (hint != NULL) fprintf(stderr, "     %s= dica:%s %s\n", green, reset, hint);
}

static inline void spike_fail(int code, int line, const char* message) {
    spike_flush();
    spike_print_error(code, line, message);
    fflush(stderr);
    spike_cleanup();
    exit(SPIKE_EXIT_RUNTIME_ERROR);
}

#endif
