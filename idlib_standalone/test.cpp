#include <stdio.h>
#include "Lib.h"

int main() {
    printf("Testing idlib_standalone...\\n");
    if (Swap_IsBigEndian()) {
        printf("Big endian machine.\\n");
    } else {
        printf("Little endian machine.\\n");
    }
    return 0;
}
