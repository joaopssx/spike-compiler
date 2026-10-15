#include <cstdio>

int main() {
    for (int i = 1; i <= 25; ++i) std::fprintf(stderr, "fake error line %d\n", i);
    return 1;
}
