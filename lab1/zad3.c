#include<stdio.h>
#include <stdbool.h>

const int MAX_SIZE = 3;
bool printable_buf(const void * buf, int len);
int main() {
    char nonPrintable = 22;
    char myBuff1[] = {'a', 'b', nonPrintable};
    bool result = printable_buf(myBuff1, sizeof(myBuff1)/ sizeof(char));
    printf("%d\n", result);
    return 0;
}

bool printable_buf(const void * buf, int len ) {
    char *ptr = (char *) buf;
    int i;
    for(i = 0; i < len; i++) {
        if(!((int)(*ptr) >= 32 && (int)(*ptr) <= 126)) return false;
        ptr++;
    }
    return true;
}