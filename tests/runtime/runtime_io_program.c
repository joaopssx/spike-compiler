#include "spike_runtime.h"

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "";
    spike_init();
    if (strcmp(mode, "int") == 0) {
        spike_write_int(spike_read_int(5));
    } else if (strcmp(mode, "real") == 0) {
        spike_write_real(spike_read_real(5));
    } else if (strcmp(mode, "bool") == 0) {
        spike_write_bool(spike_read_bool(5));
    } else if (strcmp(mode, "str") == 0) {
        spike_write_str(spike_read_str(5));
    } else if (strcmp(mode, "div") == 0) {
        spike_write_int(spike_div_int(1, 0, 7));
    } else if (strcmp(mode, "divreal") == 0) {
        spike_write_real(spike_div_real(1.0, 0.0, 9));
    } else {
        return 2;
    }
    spike_write_newline();
    spike_flush();
    return 0;
}
