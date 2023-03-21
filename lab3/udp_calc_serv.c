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
#include <limits.h>

bool validateData(const char * buff, size_t len);
int32_t processData(const char* buff, size_t len);

int main(int argc, char *argv[])
{
    int lst_sock;   // gniazdko nasłuchujące
    u_int16_t port = 2020;
    ssize_t cnt;    // wyniki zwracane przez read() i write() są tego typu
    const int MAX_UDP = 65536;
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
    char buff[MAX_UDP];
    socklen_t len = sizeof(cliaddr);

    bool keep_on_handling_clients = true;
    while(keep_on_handling_clients) {
        cnt = recvfrom(lst_sock, (char *)buff, MAX_UDP, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        if(cnt == -1) {
            perror("recvfrom");
            return 1;
        }

        printf("Received %ld bytes\n", cnt);
        if(!validateData(buff, cnt)) {
            memcpy(buff, "ERROR", 5);
            if(sendto(lst_sock, (char *) buff, 5, MSG_WAITALL, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1) {
            perror("sendto");
            return 1;
            }
        } else {
            processData(buff, cnt);
        }
    }

    if (close(lst_sock) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}

bool validateData(const char * buff, size_t len) {
    int32_t digit;
    int i;
    if(isspace(buff[0]) || buff[0] == 43 || buff[0] == 45) 
        return false;
    char allowedCharacters = "0123456789+-\r\n";
    for(i = 0; i < len-2; i++) {
      if(strchr(allowedCharacters, buff[i]) == NULL)
        return false;
    }
    if (!(strstr(buff, "\r\n") != NULL || strstr(buff, "\n") != NULL)) {
       return false;
    }
    return true;
}

int32_t processData(const char* buff, size_t len) {
    int32_t result = 0;
    int32_t curr_numb = 0;
    int prev_action = 1;
    int i;
    for (i = 0; i < len; i++) {
        if (isdigit(buff[i])) {
            curr_numb = curr_numb * 10 + (buff[i] - '0');
        }
        else if (buff[i] == '+') {
            result += prev_action * curr_numb;
            prev_action = 1;
            curr_numb = 0;
        }
        else if (buff[i] == '-') {
            result += prev_action * curr_numb;
            prev_action = -1;
            curr_numb = 0;
        }
        if(buff[i] == '\r' || buff[i] == '\n') break;
    }
    // Add the last processed number to result
    result += prev_action * curr_numb;
    return result;
}