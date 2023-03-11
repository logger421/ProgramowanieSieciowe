#include <stdio.h>

int main(void) {
    printf("%s: %d", "Hello, world!\r\n", sizeof("Hello, world!\r\n"));
    return 0;
}