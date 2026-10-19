#include "spike_runtime.h"

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "";
    spike_init();
    if (strcmp(mode, "int") == 0) {
        spike_write_int(spike_read_int(5, "x"));
    } else if (strcmp(mode, "real") == 0) {
        spike_write_real(spike_read_real(5, "x"));
    } else if (strcmp(mode, "bool") == 0) {
        spike_write_bool(spike_read_bool(5, "x"));
    } else if (strcmp(mode, "str") == 0) {
        spike_write_str(spike_read_str(5, "x"));
    } else if (strcmp(mode, "div") == 0) {
        spike_write_int(spike_div_int(1, 0, 7));
    } else if (strcmp(mode, "index") == 0) {
        spike_write_int((spike_int)spike_index(11, 1, 5, 13, "notas"));
    } else if (strcmp(mode, "index2") == 0) {
        spike_write_int((spike_int)spike_index_dim(5, 1, 4, 14, "m", 2));
    } else if (strcmp(mode, "depth") == 0) {
        for (;;) spike_enter(21);
    } else if (strcmp(mode, "noreturn") == 0) {
        spike_missing_return(22, "sinal");
    } else if (strcmp(mode, "pow") == 0) {
        spike_write_real(spike_pow(0.0, -1.0, 11));
    } else if (strcmp(mode, "powneg") == 0) {
        spike_write_real(spike_pow(-8.0, 0.5, 12));
    } else if (strcmp(mode, "divreal") == 0) {
        spike_write_real(spike_div_real(1.0, 0.0, 9));
    } else {
        return 2;
    }
    spike_write_newline();
    spike_flush();
    return 0;
}
