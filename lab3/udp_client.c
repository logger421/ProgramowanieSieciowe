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
#include <sys/time.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <portNumber> <serverAddres>\n", argv[0]);
        return 1;
    }

    int sockfd;
    ssize_t cnt;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_addr = { .s_addr = inet_addr(argv[2]) },   // 127.0.0.1
        .sin_port = htons(atoi(argv[1]))
    };

    struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof timeout) < 0)
        error("setsockopt failed\n");

    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                sizeof timeout) < 0)
        error("setsockopt failed\n");
        
    struct sockaddr_in empty_datagram;
    memset(&empty_datagram, 0, sizeof(empty_datagram));

    if(sendto(sockfd, (const struct sockaddr *) &empty_datagram, sizeof(empty_datagram),
        MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
            perror("sendto");
            return 1;
    }
    puts("Send empty datagram");
    char buff[32];
    socklen_t len;
    
    cnt = recvfrom(sockfd, (char *) buff, 32, MSG_WAITALL, (struct sockaddr *) &serv_addr, &len);
    int i;
    for(i = 0; i <= cnt; i++) {
        if(((int)buff[i]>=32 && (int)buff[i]<=126)) {
            printf("%c", buff[i]);
        }
        else {
            printf("?");
        }    
    }

    if(cnt == -1) {
        perror("recvfrom");
        return 1;
    }
    puts("\n");
    if (close(sockfd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}