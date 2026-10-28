#ifndef SPIKE_RUNTIME_CALLS_H
#define SPIKE_RUNTIME_CALLS_H

#include <stdint.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/resource.h>
#else
#include <windows.h>
#endif

#define SPIKE_MAX_CALL_DEPTH 10000
#define SPIKE_SAVED_CALLS 64

#ifndef SPIKE_R_OUT_OF_MEMORY
#define SPIKE_R_OUT_OF_MEMORY 8
#endif

#ifndef SPIKE_R_MISSING_RETURN
#define SPIKE_R_MISSING_RETURN 9
#endif

static inline void spike_fail(int code, int line, const char* message);

typedef struct spike_call {
    const char* name;
    int line;
    int depth;
} spike_call;

static int spike_call_depth = 0;
static spike_call spike_calls[SPIKE_SAVED_CALLS];
static uintptr_t spike_stack_start = 0;
static uintptr_t spike_stack_budget = 0;

/* Bytes of stack below the first call. Recursion stops at 3/4 of it, before a crash. */
static inline uintptr_t spike_stack_size(uintptr_t here) {
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION info;
    if (VirtualQuery((const void*)here, &info, sizeof info) != 0)
        return here - (uintptr_t)info.AllocationBase;
#else
    struct rlimit limit;
    if (getrlimit(RLIMIT_STACK, &limit) == 0 && limit.rlim_cur != RLIM_INFINITY)
        return (uintptr_t)limit.rlim_cur;
#endif
    return (uintptr_t)8 * 1024 * 1024;
}

/* Slots are reused every 64 levels; depth tells whether a slot still holds that level. */
static inline void spike_enter(const char* name, int line) {
    char marker;
    uintptr_t here = (uintptr_t)&marker;
    uintptr_t used;
    spike_call* slot;
    if (spike_stack_start == 0) {
        spike_stack_start = here;
        spike_stack_budget = spike_stack_size(here) / 4 * 3;
    }
    used = spike_stack_start > here ? spike_stack_start - here : here - spike_stack_start;
    ++spike_call_depth;
    slot = &spike_calls[(spike_call_depth - 1) % SPIKE_SAVED_CALLS];
    slot->name = name;
    slot->line = line;
    slot->depth = spike_call_depth;
    if (spike_call_depth > SPIKE_MAX_CALL_DEPTH)
        spike_fail(SPIKE_R_OUT_OF_MEMORY, line,
                   "recursão profunda demais (limite de 10000 chamadas), verifique a condição "
                   "de parada");
    if (used > spike_stack_budget)
        spike_fail(SPIKE_R_OUT_OF_MEMORY, line,
                   "recursão profunda demais (a memória para chamadas acabou), verifique a "
                   "condição de parada");
}

static inline void spike_leave(void) {
    if (spike_call_depth > 0) --spike_call_depth;
}

static inline void spike_missing_return(int line, const char* name) {
    char message[200];
    snprintf(message, sizeof message, "a função '%s' terminou sem retornar um valor", name);
    spike_fail(SPIKE_R_MISSING_RETURN, line, message);
}

#endif
