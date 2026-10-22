#ifndef SPIKE_RUNTIME_CORE_H
#define SPIKE_RUNTIME_CORE_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

typedef long long spike_int;
typedef double spike_real;
typedef int spike_bool;
typedef const char *spike_str;

#define SPIKE_R_DIV_ZERO 1
#define SPIKE_R_INDEX 2
#define SPIKE_R_BAD_INPUT 3
#define SPIKE_R_CONVERSION 4
#define SPIKE_R_MATH_DOMAIN 5
#define SPIKE_R_ZERO_STEP 6
#define SPIKE_R_END_OF_INPUT 7
#define SPIKE_R_OUT_OF_MEMORY 8
#define SPIKE_R_MISSING_RETURN 9
#define SPIKE_R_RANDOM_RANGE 10
#define SPIKE_R_TEXT_RANGE 11
#define SPIKE_EXIT_RUNTIME_ERROR 101

typedef struct spike_block
{
    struct spike_block *next;
} spike_block;

static spike_block *spike_blocks = NULL;

#ifdef _WIN32
static UINT spike_saved_output_cp = 0;
static UINT spike_saved_input_cp = 0;
#endif

static inline void spike_flush(void)
{
    fflush(stdout);
}

static inline void spike_cleanup(void)
{
    while (spike_blocks != NULL)
    {
        spike_block *next = spike_blocks->next;
        free(spike_blocks);
        spike_blocks = next;
    }
#ifdef _WIN32
    if (spike_saved_output_cp != 0)
        SetConsoleOutputCP(spike_saved_output_cp);
    if (spike_saved_input_cp != 0)
        SetConsoleCP(spike_saved_input_cp);
    spike_saved_output_cp = 0;
    spike_saved_input_cp = 0;
#endif
}

static inline void spike_fail(int code, int line, const char *message)
{
    spike_flush();
    fprintf(stderr, "erro de execução[R%03d]: %s\n", code, message);
    if (line > 0)
        fprintf(stderr, "  --> linha %d\n", line);
    fflush(stderr);
    spike_cleanup();
    exit(SPIKE_EXIT_RUNTIME_ERROR);
}

static inline void spike_init(void)
{
#ifdef _WIN32
    UINT output_cp = GetConsoleOutputCP();
    UINT input_cp = GetConsoleCP();
    if (output_cp != 0 && output_cp != CP_UTF8 && SetConsoleOutputCP(CP_UTF8))
    {
        spike_saved_output_cp = output_cp;
    }
    if (input_cp != 0 && input_cp != CP_UTF8 && SetConsoleCP(CP_UTF8))
    {
        spike_saved_input_cp = input_cp;
    }
#endif
    atexit(spike_cleanup);
}

static inline char *spike_alloc(size_t size)
{
    spike_block *block = (spike_block *)malloc(sizeof(spike_block) + size);
    if (block == NULL)
        spike_fail(SPIKE_R_OUT_OF_MEMORY, 0, "memória insuficiente");
    block->next = spike_blocks;
    spike_blocks = block;
    return (char *)(block + 1);
}

static inline spike_str spike_str_copy(spike_str s)
{
    size_t len = strlen(s);
    char *out = spike_alloc(len + 1);
    memcpy(out, s, len + 1);
    return out;
}

static inline spike_str spike_str_concat(spike_str a, spike_str b)
{
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    char *out = spike_alloc(len_a + len_b + 1);
    memcpy(out, a, len_a);
    memcpy(out + len_a, b, len_b + 1);
    return out;
}

static inline spike_bool spike_str_eq(spike_str a, spike_str b)
{
    return strcmp(a, b) == 0;
}

static inline int spike_str_cmp(spike_str a, spike_str b)
{
    int result = strcmp(a, b);
    return (result > 0) - (result < 0);
}

static inline spike_int spike_add_int(spike_int a, spike_int b)
{
    return (spike_int)((unsigned long long)a + (unsigned long long)b);
}

static inline spike_int spike_sub_int(spike_int a, spike_int b)
{
    return (spike_int)((unsigned long long)a - (unsigned long long)b);
}

static inline spike_int spike_mul_int(spike_int a, spike_int b)
{
    return (spike_int)((unsigned long long)a * (unsigned long long)b);
}

static inline spike_int spike_div_int(spike_int a, spike_int b, int line)
{
    if (b == 0)
        spike_fail(SPIKE_R_DIV_ZERO, line, "divisão por zero");
    if (b == -1)
        return (spike_int)(0ULL - (unsigned long long)a);
    return a / b;
}

static inline spike_int spike_mod_int(spike_int a, spike_int b, int line)
{
    if (b == 0)
        spike_fail(SPIKE_R_DIV_ZERO, line, "divisão por zero");
    if (b == -1)
        return 0;
    return a % b;
}

static inline spike_real spike_div_real(spike_real a, spike_real b, int line)
{
    if (b == 0.0)
        spike_fail(SPIKE_R_DIV_ZERO, line, "divisão por zero");
    return a / b;
}

static inline spike_real spike_pow(spike_real base, spike_real exponent, int line)
{
    spike_real result = pow(base, exponent);
    if (isnan(result) || isinf(result))
    {
        spike_fail(SPIKE_R_MATH_DOMAIN, line, "resultado inválido na potência");
    }
    return result;
}

static inline size_t spike_index(spike_int index, spike_int low, spike_int high, int line,
                                 const char* name) {
    if (index < low || index > high) {
        char message[200];
        snprintf(message, sizeof message, "índice %lld fora do vetor '%s' (válido: %lld a %lld)",
                 index, name, low, high);
        spike_fail(SPIKE_R_INDEX, line, message);
    }
    return (size_t)(index - low);
}

static inline size_t spike_index_dim(spike_int index, spike_int low, spike_int high, int line,
                                     const char* name, int dimension) {
    if (index < low || index > high) {
        char message[200];
        snprintf(message, sizeof message,
                 "índice %lld fora da dimensão %d de '%s' (válido: %lld a %lld)", index,
                 dimension, name, low, high);
        spike_fail(SPIKE_R_INDEX, line, message);
    }
    return (size_t)(index - low);
}

static inline void spike_fill_str(spike_str* items, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) items[i] = "";
}

static inline void spike_check_step(spike_int step, int line)
{
    if (step == 0)
        spike_fail(SPIKE_R_ZERO_STEP, line, "o passo do 'para' não pode ser zero");
}

static inline spike_bool spike_for_advance(spike_int *counter, spike_int step)
{
    if (step > 0 ? *counter > LLONG_MAX - step : *counter < LLONG_MIN - step)
        return 0;
    *counter += step;
    return 1;
}

#endif
