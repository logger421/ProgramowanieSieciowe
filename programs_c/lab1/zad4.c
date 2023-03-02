#include <stdio.h>
#include <stdbool.h>
// only if for loop uncommented.
// #include <string.h>

bool printable_str(const char * buf);

int main() {
    const char * myStr = "Hello world\0";
    printf("%s\n", myStr);
    bool result = printable_str(myStr);
    printf("%s\n", result ? "true" : "false");
    return 0;
}

bool printable_str(const char * buf) {
    size_t i = 0;
    // strings in C are guaranteed to be null-terminated
    // size_t len = strlen(buf);
    // for(i = 0; i < len; i++) {
    //     if(!((int)buf[i] >= 32 && (int)buf[i] <= 126)) return false;
    // }
    for(i = 0; buf[i] != '\0'; i++) {
        if(!((int)buf[i] >= 32 && (int)buf[i] <= 126)) return false;
    }
    return true;
}