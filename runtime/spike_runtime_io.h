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

static inline void spike_input_error(int line, const char* expected, const char* received) {
    char shown[48];
    char message[160];
    size_t length = strlen(received);
    if (length == 0) {
        snprintf(message, sizeof message, "esperava %s, mas recebeu uma linha vazia", expected);
        spike_fail(SPIKE_R_BAD_INPUT, line, message);
    }
    if (length > 40) {
        length = 40;
        while (length > 0 && ((unsigned char)received[length] & 0xC0) == 0x80) --length;
        memcpy(shown, received, length);
        memcpy(shown + length, "...", 4);
    } else {
        memcpy(shown, received, length + 1);
    }
    snprintf(message, sizeof message, "esperava %s, mas recebeu '%s'", expected, shown);
    spike_fail(SPIKE_R_BAD_INPUT, line, message);
}

static int spike_input_started = 0;

static inline void spike_skip_input_bom(void) {
    int c;
    if (spike_input_started) return;
    spike_input_started = 1;
    c = getchar();
    if (c != 0xEF) {
        if (c != EOF) ungetc(c, stdin);
        return;
    }
    if (getchar() == 0xBB && getchar() == 0xBF) return;
    spike_fail(SPIKE_R_BAD_INPUT, 0, "a entrada começa com bytes inválidos");
}

static inline char* spike_read_line(int line, const char* name) {
    size_t capacity = 64;
    size_t length = 0;
    char* buffer;
    int c;
    spike_flush();
    buffer = spike_alloc(capacity);
    spike_skip_input_bom();
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
        char message[128];
        snprintf(message, sizeof message, "a entrada acabou antes de leia() conseguir ler '%s'",
                 name);
        spike_fail(SPIKE_R_END_OF_INPUT, line, message);
    }
    if (length > 0 && buffer[length - 1] == '\r') --length;
    buffer[length] = '\0';
    return buffer;
}

static inline spike_int spike_read_int(int line, const char* name) {
    spike_int value = 0;
    const char* text = spike_read_line(line, name);
    if (!spike_parse_int(text, &value)) spike_input_error(line, "um número inteiro", text);
    return value;
}

static inline spike_real spike_read_real(int line, const char* name) {
    spike_real value = 0.0;
    const char* text = spike_read_line(line, name);
    if (!spike_parse_real(text, &value)) spike_input_error(line, "um número", text);
    return value;
}

static inline spike_str spike_read_str(int line, const char* name) {
    return spike_read_line(line, name);
}

static inline spike_bool spike_read_bool(int line, const char* name) {
    spike_bool value = 0;
    const char* text = spike_read_line(line, name);
    if (!spike_parse_bool(text, &value)) {
        spike_input_error(line, "verdadeiro ou falso (ou v, f, sim, nao)", text);
    }
    return value;
}

#endif
