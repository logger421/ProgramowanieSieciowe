// Szkielet klienta TCP/IPv4.
//
// Po podmienieniu SOCK_STREAM na SOCK_DGRAM staje się on szkieletem klienta
// UDP/IPv4 korzystającego z gniazdka działającego w trybie połączeniowym.

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <portNumber> <serverAddres>\n", argv[0]);
        return 1;
    }

    int sock;
    int rc;         // "rc" to skrót słów "result code"
    ssize_t cnt;    // wyniki zwracane przez read() i write() są tego typu
    u_int16_t port = atoi(argv[1]);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = { .s_addr = inet_addr(argv[2]) },   // 127.0.0.1
        .sin_port = htons(port)
    };
    rc = connect(sock, (struct sockaddr *) & addr, sizeof(addr));
    if (rc == -1) {
        perror("connect");
        return 1;
    }

    unsigned char buf[24];
    cnt = read(sock, buf, 24);
    if (cnt == -1) {
        perror("read");
        return 1;
    }
    printf("read %zi bytes\n", cnt);
    int i;
    for(i = 0; i <= cnt; i++) {
        if(((int)buf[i]>=32 && (int)buf[i]<=126)) 
            printf("%c", buf[i]);
        else
            printf("?");
    }
    puts("\n");
    rc = close(sock);
    if (rc == -1) {
        perror("close");
        return 1;
    }

    return 0;
}