// Szkielet serwera UDP/IPv4.

#define _POSIX_C_SOURCE 200809L
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>

const int MAX_UDP = 65536;
int OVERFLOW_ERR = 0;

bool validateData(const char * buff, size_t len);
int32_t processData(const char* buff, size_t len);

int main(int argc, char *argv[])
{
    
    int32_t result;
    u_int16_t port = 2020;
    ssize_t cnt;    // wyniki zwracane przez read() i write() są tego typu
    int lst_sock;   // gniazdko nasłuchujące    
    char buff[MAX_UDP];
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
    
    if (bind(lst_sock, (struct sockaddr *) & addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }
    


    bool keep_on_handling_clients = true;
    while(keep_on_handling_clients) {
        // struktura reprezentująca odebrany adres klienta
        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));

        socklen_t len = sizeof(cliaddr);

        cnt = recvfrom(lst_sock, (char *)buff, MAX_UDP, 0, (struct sockaddr *) &cliaddr, &len);
        if(cnt == -1) {
            perror("recvfrom");
            return 1;
        }

        printf("Received %ld bytes\n", cnt);

        if(!validateData(buff, cnt)) {
            memcpy(buff, "ERROR", 5);
            if(sendto(lst_sock, (char *) buff, 5, 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1) {
                perror("sendto");
                return 1;
            }
        } else {
            result = processData(buff, cnt);
            if(OVERFLOW_ERR == 1) {
                fprintf(stderr, "overflow/underflow\n");
                memcpy(buff, "ERROR", 5);
                if(sendto(lst_sock, (char *) buff, 5, 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1) {
                    perror("sendto");
                    return 1;
                }

                OVERFLOW_ERR = 0;
                continue;
            }
            if((cnt = sprintf(buff, "%d", result)) == -1) {
                perror("sprintf");
                return 1;
            }
            if(sendto(lst_sock, (char *) buff, cnt, 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1) {
                perror("sendto");
                return 1;
            }
        }
    }

    if (close(lst_sock) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}

bool validateData(const char * buff, size_t len) {
    char allowedCharacters[] = "0123456789+-";
    if(!isdigit(buff[0]))
        return false;
    int i;
    for(i = 0; i < len; i++) {
        if(strchr(allowedCharacters, buff[i]) == NULL)
            return false;
        if(isdigit(buff[i]) && (buff[i+1] == 43 || buff[i+1] == 45) && !isdigit(i+2))
            return false;
    }
    return true;
}

int32_t processData(const char* buff, size_t len) {
    int64_t result = 0;
    int64_t curr_numb = 0;
    int prev_action = 1;
    int i;
    for (i = 0; i < len; i++) {
        if (isdigit(buff[i])) {
            curr_numb = curr_numb * 10 + (buff[i] - '0');
        }
        else if (buff[i] == '+') {
            result += prev_action * curr_numb;
            if(result > INT32_MAX) {
                OVERFLOW_ERR = 1;
                return (int32_t) result;
            }
            prev_action = 1;
            curr_numb = 0;
        }
        else if (buff[i] == '-') {
            result += prev_action * curr_numb;
            if(result < INT32_MIN) {
                OVERFLOW_ERR = 1;
                return (int32_t) result;
            }
            prev_action = -1;
            curr_numb = 0;
        }
    }
    result += prev_action * curr_numb;
    if(result < INT32_MIN) {
        OVERFLOW_ERR = 1;
        return (int32_t) result;
    }
    if(result > INT32_MAX) {
        OVERFLOW_ERR = 1;
        return (int32_t) result;
    }
    return (int32_t) result;
}
