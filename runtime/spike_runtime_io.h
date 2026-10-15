#ifndef SPIKE_RUNTIME_IO_H
#define SPIKE_RUNTIME_IO_H

static inline void spike_format_real(spike_real value, char* buffer, size_t size) {
    snprintf(buffer, size, "%.10g", value);
    if (strcmp(buffer, "-0") == 0) snprintf(buffer, size, "0");
}

static inline void spike_write_int(spike_int value) {
    printf("%lld", value);
}

static inline void spike_write_real(spike_real value) {
    char buffer[64];
    spike_format_real(value, buffer, sizeof buffer);
    fputs(buffer, stdout);
}

static inline void spike_write_str(spike_str value) {
    fputs(value, stdout);
}

static inline void spike_write_bool(spike_bool value) {
    fputs(value ? "verdadeiro" : "falso", stdout);
}

static inline void spike_write_newline(void) {
    putchar('\n');
}

static inline char* spike_trim(char* text) {
    char* end;
    while (isspace((unsigned char)*text)) ++text;
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) --end;
    *end = '\0';
    return text;
}

static inline spike_bool spike_parse_int(const char* text, spike_int* out) {
    char buffer[64];
    char* trimmed;
    char* end;
    size_t i;
    long long value;
    if (strlen(text) >= sizeof buffer) return 0;
    strcpy(buffer, text);
    trimmed = spike_trim(buffer);
    i = (trimmed[0] == '-' || trimmed[0] == '+') ? 1 : 0;
    if (trimmed[i] == '\0') return 0;
    for (; trimmed[i] != '\0'; ++i) {
        if (!isdigit((unsigned char)trimmed[i])) return 0;
    }
    errno = 0;
    value = strtoll(trimmed, &end, 10);
    if (errno == ERANGE || *end != '\0') return 0;
    *out = value;
    return 1;
}

static inline spike_bool spike_parse_real(const char* text, spike_real* out) {
    char buffer[128];
    char* trimmed;
    char* end;
    size_t i;
    int digits = 0;
    int points = 0;
    if (strlen(text) >= sizeof buffer) return 0;
    strcpy(buffer, text);
    trimmed = spike_trim(buffer);
    i = (trimmed[0] == '-' || trimmed[0] == '+') ? 1 : 0;
    for (; trimmed[i] != '\0'; ++i) {
        if (trimmed[i] == ',') trimmed[i] = '.';
        if (trimmed[i] == '.') {
            ++points;
        } else if (isdigit((unsigned char)trimmed[i])) {
            ++digits;
        } else {
            return 0;
        }
    }
    if (digits == 0 || points > 1) return 0;
    errno = 0;
    *out = strtod(trimmed, &end);
    return errno != ERANGE && *end == '\0';
}

static inline spike_bool spike_parse_bool(const char* text, spike_bool* out) {
    static const char* const yes[] = {"verdadeiro", "v", "sim"};
    static const char* const no[] = {"falso", "f", "nao"};
    char buffer[32];
    char* trimmed;
    size_t i;
    if (strlen(text) >= sizeof buffer) return 0;
    strcpy(buffer, text);
    trimmed = spike_trim(buffer);
    for (i = 0; trimmed[i] != '\0'; ++i) {
        trimmed[i] = (char)tolower((unsigned char)trimmed[i]);
    }
    for (i = 0; i < 3; ++i) {
        if (strcmp(trimmed, yes[i]) == 0) { *out = 1; return 1; }
        if (strcmp(trimmed, no[i]) == 0) { *out = 0; return 1; }
    }
    return 0;
}

static inline char* spike_read_line(int line) {
    size_t capacity = 64;
    size_t length = 0;
    char* buffer;
    int c;
    spike_flush();
    buffer = spike_alloc(capacity);
    while ((c = getchar()) != EOF && c != '\n') {
        if (length + 1 == capacity) {
            char* bigger = spike_alloc(capacity * 2);
            memcpy(bigger, buffer, length);
            buffer = bigger;
            capacity *= 2;
        }
        buffer[length++] = (char)c;
    }
    if (c == EOF && length == 0) {
        spike_fail(SPIKE_R_END_OF_INPUT, line, "a entrada terminou antes do 'leia'");
    }
    if (length > 0 && buffer[length - 1] == '\r') --length;
    buffer[length] = '\0';
    return buffer;
}

static inline spike_int spike_read_int(int line) {
    spike_int value = 0;
    if (!spike_parse_int(spike_read_line(line), &value)) {
        spike_fail(SPIKE_R_BAD_INPUT, line, "o valor digitado não é um número inteiro");
    }
    return value;
}

static inline spike_real spike_read_real(int line) {
    spike_real value = 0.0;
    if (!spike_parse_real(spike_read_line(line), &value)) {
        spike_fail(SPIKE_R_BAD_INPUT, line, "o valor digitado não é um número");
    }
    return value;
}

static inline spike_str spike_read_str(int line) {
    return spike_read_line(line);
}

static inline spike_bool spike_read_bool(int line) {
    spike_bool value = 0;
    if (!spike_parse_bool(spike_read_line(line), &value)) {
        spike_fail(SPIKE_R_BAD_INPUT, line,
                   "digite verdadeiro ou falso (também valem v, f, sim e nao)");
    }
    return value;
}

#endif
