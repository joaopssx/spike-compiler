#ifndef SPIKE_RUNTIME_MATH_H
#define SPIKE_RUNTIME_MATH_H

#include <time.h>

static inline spike_int spike_abs_int(spike_int value)
{
    return value < 0 ? (spike_int)(0ULL - (unsigned long long)value) : value;
}

static inline spike_real spike_sqrt(spike_real value, int line)
{
    if (value < 0)
        spike_fail(SPIKE_R_MATH_DOMAIN, line, "raiz de número negativo");
    return sqrt(value);
}

static inline spike_real spike_log(spike_real value, int line)
{
    if (!(value > 0))
        spike_fail(SPIKE_R_MATH_DOMAIN, line, "logaritmo de número que não é positivo");
    return log10(value);
}

static inline spike_int spike_real_to_int(spike_real value, int line)
{
    if (isnan(value))
        spike_fail(SPIKE_R_CONVERSION, line, "o valor não é um número e não pode virar inteiro");
    if (value >= 9223372036854775808.0 || value < -9223372036854775808.0)
        spike_fail(SPIKE_R_CONVERSION, line, "número grande demais para virar inteiro");
    return (spike_int)value;
}

static inline spike_int spike_round(spike_real value, int line)
{
    return spike_real_to_int(round(value), line);
}

static inline spike_int spike_trunc(spike_real value, int line)
{
    return spike_real_to_int(trunc(value), line);
}

static inline spike_int spike_floor(spike_real value, int line)
{
    return spike_real_to_int(floor(value), line);
}

static inline spike_int spike_ceil(spike_real value, int line)
{
    return spike_real_to_int(ceil(value), line);
}

static unsigned long long spike_random_state = 0;
static int spike_random_ready = 0;

static inline void spike_seed_random(unsigned long long seed)
{
    spike_random_state = seed;
    spike_random_ready = 1;
}

/* splitmix64: the same sequence on every system, so SPIKE_SEED gives repeatable runs. */
static inline unsigned long long spike_random_next(void)
{
    unsigned long long z;
    if (!spike_random_ready)
    {
        const char *seed = getenv("SPIKE_SEED");
        spike_seed_random(seed != NULL && seed[0] != '\0' ? strtoull(seed, NULL, 10)
                                                          : (unsigned long long)time(NULL));
    }
    spike_random_state += 0x9E3779B97F4A7C15ULL;
    z = spike_random_state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static inline spike_int spike_random(spike_int low, spike_int high, int line)
{
    unsigned long long span;
    unsigned long long limit;
    unsigned long long value;
    if (low > high)
    {
        char message[200];
        snprintf(message, sizeof message,
                 "aleatorio(%lld, %lld): o primeiro número precisa ser menor ou igual ao segundo",
                 low, high);
        spike_fail(SPIKE_R_RANDOM_RANGE, line, message);
    }
    span = (unsigned long long)high - (unsigned long long)low + 1ULL;
    if (span == 0)
        return (spike_int)spike_random_next();
    limit = ULLONG_MAX - ULLONG_MAX % span;
    do
        value = spike_random_next();
    while (value >= limit);
    return (spike_int)((unsigned long long)low + value % span);
}

#endif
