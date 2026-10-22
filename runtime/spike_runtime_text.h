#ifndef SPIKE_RUNTIME_TEXT_H
#define SPIKE_RUNTIME_TEXT_H

/* Bytes of the character at s. A byte that does not start a valid UTF-8 sequence counts as one
   character, so broken text never makes these functions skip or overrun. */
static inline size_t spike_utf8_size(const char *s)
{
    const unsigned char *u = (const unsigned char *)s;
    size_t need;
    size_t i;
    if (u[0] < 0x80)
        return 1;
    if (u[0] >= 0xC2 && u[0] <= 0xDF)
        need = 2;
    else if (u[0] >= 0xE0 && u[0] <= 0xEF)
        need = 3;
    else if (u[0] >= 0xF0 && u[0] <= 0xF4)
        need = 4;
    else
        return 1;
    for (i = 1; i < need; ++i)
        if ((u[i] & 0xC0) != 0x80)
            return 1;
    return need;
}

static inline spike_int spike_str_length(spike_str text)
{
    spike_int count = 0;
    while (*text != '\0')
    {
        text += spike_utf8_size(text);
        ++count;
    }
    return count;
}

static inline spike_str spike_str_change_case(spike_str text, int upper)
{
    size_t length = strlen(text);
    char *out = spike_alloc(length + 1);
    size_t i = 0;
    memcpy(out, text, length + 1);
    while (i < length)
    {
        unsigned char *c = (unsigned char *)out + i;
        size_t size = spike_utf8_size(out + i);
        if (size == 1)
            *c = (unsigned char)(upper ? toupper(*c) : tolower(*c));
        else if (size == 2 && c[0] == 0xC3)
        {
            /* U+00E0..U+00FE are the lower case of U+00C0..U+00DE, except the signs x and / */
            if (upper && c[1] >= 0xA0 && c[1] <= 0xBE && c[1] != 0xB7)
                c[1] = (unsigned char)(c[1] - 0x20);
            else if (!upper && c[1] >= 0x80 && c[1] <= 0x9E && c[1] != 0x97)
                c[1] = (unsigned char)(c[1] + 0x20);
        }
        i += size;
    }
    return out;
}

static inline spike_str spike_str_upper(spike_str text)
{
    return spike_str_change_case(text, 1);
}

static inline spike_str spike_str_lower(spike_str text)
{
    return spike_str_change_case(text, 0);
}

static inline spike_str spike_str_slice(spike_str text, spike_int start, spike_int count, int line)
{
    const char *begin = text;
    const char *end;
    spike_int i;
    char *out;
    if (start < 1)
    {
        char message[160];
        snprintf(message, sizeof message,
                 "copia: a posição inicial precisa ser 1 ou mais, mas é %lld", start);
        spike_fail(SPIKE_R_TEXT_RANGE, line, message);
    }
    for (i = 1; i < start && *begin != '\0'; ++i)
        begin += spike_utf8_size(begin);
    end = begin;
    for (i = 0; i < count && *end != '\0'; ++i)
        end += spike_utf8_size(end);
    out = spike_alloc((size_t)(end - begin) + 1);
    memcpy(out, begin, (size_t)(end - begin));
    out[end - begin] = '\0';
    return out;
}

static inline spike_int spike_str_find(spike_str needle, spike_str text)
{
    const char *found;
    spike_int position = 1;
    if (*needle == '\0')
        return 0;
    found = strstr(text, needle);
    if (found == NULL)
        return 0;
    while (text < found)
    {
        text += spike_utf8_size(text);
        ++position;
    }
    return position;
}

static inline spike_str spike_int_to_str(spike_int value)
{
    char buffer[32];
    snprintf(buffer, sizeof buffer, "%lld", value);
    return spike_str_copy(buffer);
}

static inline spike_str spike_real_to_str(spike_real value)
{
    char buffer[64];
    spike_format_real(value, buffer, sizeof buffer);
    return spike_str_copy(buffer);
}

static inline spike_str spike_bool_to_str(spike_bool value)
{
    return value ? "verdadeiro" : "falso";
}

static inline void spike_conversion_error(spike_str text, const char *target, int line)
{
    char shown[48];
    char message[160];
    if (*text == '\0')
    {
        snprintf(message, sizeof message, "não foi possível converter um texto vazio para %s",
                 target);
    }
    else
    {
        spike_shorten(text, shown);
        snprintf(message, sizeof message, "não foi possível converter '%s' para %s", shown,
                 target);
    }
    spike_fail(SPIKE_R_CONVERSION, line, message);
}

static inline spike_int spike_str_to_int(spike_str text, int line)
{
    spike_int value = 0;
    if (!spike_parse_int(text, &value))
        spike_conversion_error(text, "inteiro", line);
    return value;
}

static inline spike_real spike_str_to_real(spike_str text, int line)
{
    spike_real value = 0.0;
    if (!spike_parse_real(text, &value))
        spike_conversion_error(text, "real", line);
    return value;
}

static inline spike_bool spike_str_is_number(spike_str text)
{
    spike_int whole;
    spike_real real;
    return spike_parse_int(text, &whole) || spike_parse_real(text, &real);
}

static inline spike_int spike_ord(spike_str text, int line)
{
    const unsigned char *u = (const unsigned char *)text;
    size_t size;
    if (*text == '\0')
        spike_fail(SPIKE_R_TEXT_RANGE, line, "ord: o texto está vazio");
    size = spike_utf8_size(text);
    if (size == 1)
        return u[0];
    if (size == 2)
        return ((spike_int)(u[0] & 0x1F) << 6) | (u[1] & 0x3F);
    if (size == 3)
        return ((spike_int)(u[0] & 0x0F) << 12) | ((spike_int)(u[1] & 0x3F) << 6) | (u[2] & 0x3F);
    return ((spike_int)(u[0] & 0x07) << 18) | ((spike_int)(u[1] & 0x3F) << 12) |
           ((spike_int)(u[2] & 0x3F) << 6) | (u[3] & 0x3F);
}

static inline spike_str spike_chr(spike_int code, int line)
{
    char *out;
    if (code < 1 || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF))
    {
        char message[160];
        snprintf(message, sizeof message,
                 "chr: %lld não é o código de um caractere (válido: 1 a 1114111)", code);
        spike_fail(SPIKE_R_TEXT_RANGE, line, message);
    }
    out = spike_alloc(5);
    if (code < 0x80)
    {
        out[0] = (char)code;
        out[1] = '\0';
    }
    else if (code < 0x800)
    {
        out[0] = (char)(0xC0 | (code >> 6));
        out[1] = (char)(0x80 | (code & 0x3F));
        out[2] = '\0';
    }
    else if (code < 0x10000)
    {
        out[0] = (char)(0xE0 | (code >> 12));
        out[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        out[2] = (char)(0x80 | (code & 0x3F));
        out[3] = '\0';
    }
    else
    {
        out[0] = (char)(0xF0 | (code >> 18));
        out[1] = (char)(0x80 | ((code >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((code >> 6) & 0x3F));
        out[3] = (char)(0x80 | (code & 0x3F));
        out[4] = '\0';
    }
    return out;
}

#endif
