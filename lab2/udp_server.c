// Szkielet serwera UDP/IPv4.

#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    // assure that user passed 2 arguments
    if (argc != 2) {
        printf("Usage: %s <portNumber>\n", argv[0]);
        return 1;
    }

    int lst_sock;   // gniazdko nasłuchujące
    u_int16_t port = atoi(argv[1]);
    ssize_t cnt;    // wyniki zwracane przez read() i write() są tego typu

    // Stwórz nasluchujace gniazdo UDP
    if ( (lst_sock = socket(AF_INET,  SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return 1;
    }
    // Struktura reprezentujaca adres servera
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = { .s_addr = htonl(INADDR_ANY) },
        .sin_port = htons(port)
    };
    // struktura reprezentująca odebrany adres klienta
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));

    if (bind(lst_sock, (struct sockaddr *) & addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }
    char buff[32];
    socklen_t len = sizeof(cliaddr);

    bool keep_on_handling_clients = true;
    while(keep_on_handling_clients) {
        cnt = recvfrom(lst_sock, (char *)buff, 32, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        if(cnt == -1) {
            perror("recvfrom");
            return 1;
        }

        printf("Received %ld bytes\n", cnt);

        memcpy(buff, "Hello, world!\r\n", 15);
        if(sendto(lst_sock, (char *) buff, 15, MSG_WAITALL, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1) {
            perror("sendto");
            return 1;
        }
    }

    if (close(lst_sock) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}