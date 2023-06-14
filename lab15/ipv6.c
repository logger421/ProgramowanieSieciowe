// Przykład dydaktyczny: gniazdka IPv6.
//
// gcc -std=c99 -pedantic -Wall ipv6.c -o ipv6
//
// Program używa funkcji inet_ntop() i inet_pton(), co za tym idzie akceptuje
// tylko numeryczną postać adresów IP. Gdyby użyć getaddrinfo() byłby w stanie
// rozpoznawać również nazwy z DNS.
//
// Przy wywołaniu "./ipv6 snd6 127.0.0.1 9988" program stworzy gniazdko IPv6,
// a następnie wywoła connect() z adresem IPv4. Czym to się skończy?
//
// A dla wywołania "./ipv6 snd6 ::FFFF:127.0.0.1 9988" (adres IPv4-mapped)?

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

// Program może działać w roli nadawcy bądź odbiorcy, używając gniazdka
// sieciowego jednego z trzech typów. Wybrana rola i tryb pracy pamiętane
// są w poniższych zmiennych (ustawia je funkcja main).
//
enum {
    receiver, sender
} role;
enum {
    ipv4, ipv6, ipv6only
} mode;

// Fukcja drukująca adres gniazdka. Proszę zwrócić uwagę na sposób, w jaki
// w zależności od rodzaju adresu rzutowany jest typ wskaźnika.
//
void print_sockaddr(const char *msg, struct sockaddr_storage *ssp) {
    if (ssp->ss_family == AF_INET) {

        struct sockaddr_in *p = (struct sockaddr_in *) ssp;
        char buf[INET_ADDRSTRLEN];
        if (NULL == inet_ntop(AF_INET, &(p->sin_addr), buf, sizeof(buf))) {
            perror("inet_ntop");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "%s AF_INET addr %s port %i\n",
                msg, buf, ntohs(p->sin_port));

    } else if (ssp->ss_family == AF_INET6) {

        struct sockaddr_in6 *q = (struct sockaddr_in6 *) ssp;
        char buf[INET6_ADDRSTRLEN];
        if (NULL == inet_ntop(AF_INET6, &(q->sin6_addr), buf, sizeof(buf))) {
            perror("inet_ntop");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "%s AF_INET6 addr %s port %i\n",
                msg, buf, ntohs(q->sin6_port));

    } else {

        fprintf(stderr, "print_sockaddr: unknown family\n");
        exit(EXIT_FAILURE);

    }
}

void print_endpoints(int sock) {
    struct sockaddr_storage addr;
    socklen_t addr_len;

    addr_len = sizeof(addr);
    if (-1 == getsockname(sock, (struct sockaddr *) &addr, &addr_len)) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }
    print_sockaddr("local endpoint at", &addr);

    addr_len = sizeof(addr);
    if (-1 == getpeername(sock, (struct sockaddr *) &addr, &addr_len)) {
        perror("getpeername");
        exit(EXIT_FAILURE);
    }
    print_sockaddr("remote endpoint at", &addr);
}

// Przetwarzanie łańcuchów specyfikujących numer portu.
//
in_port_t str2port(const char *arg_port) {
    long int port;
    char *p;
    errno = 0;
    port = strtol(arg_port, &p, 10);
    if (errno != 0 || *p != '\0' || port < 1 || port > 65535) {
        fprintf(stderr, "%s is not a valid TCP port number\n", arg_port);
        exit(EXIT_FAILURE);
    }
    return port;
}

// Funkcja przygotowująca strukturę reprezentującą adres nasłuchiwania na
// wskazanym przez użytkownika numerze portu. To, czy to będzie adres typu
// AF_INET czy AF_INET6 zależy od tego, w jakim trybie działa program.
//
void make_sockaddr_for_binding(const char *arg_port,
                               struct sockaddr_storage *ssp) {
    in_port_t port = str2port(arg_port);

    memset(ssp, 0, sizeof(*ssp));
    if (mode == ipv4) {
        struct sockaddr_in *p = (struct sockaddr_in *) ssp;
        p->sin_family = AF_INET;
        p->sin_addr.s_addr = htonl(INADDR_ANY);
        p->sin_port = htons(port);
    } else {   // mode == ipv6 lub ipv6only
        struct sockaddr_in6 *q = (struct sockaddr_in6 *) ssp;
        q->sin6_family = AF_INET6;
        q->sin6_addr = in6addr_any;   // in6addr_any jest strukturą
        q->sin6_port = htons(port);
    }
}

// Funkcja przetwarzająca argumenty z linii poleceń i wypełniająca na ich
// podstawie strukturę sockaddr_in albo sockaddr_in6 w zależności od tego,
// czy użytkownik podał adres IPv4 czy IPv6.
//
void make_sockaddr_for_connecting(const char *arg_addr,
                                  const char *arg_port, struct sockaddr_storage *ssp) {
    in_port_t port = str2port(arg_port);

    // próba konwersji łańcucha z adresem jako adresu IPv4...
    struct sockaddr_in *p = (struct sockaddr_in *) ssp;
    memset(p, 0, sizeof(*p));
    if (1 == inet_pton(AF_INET, arg_addr, &(p->sin_addr))) {
        // sukces! uzupełnij pozostałe pola struktury
        p->sin_family = AF_INET;
        p->sin_port = htons(port);
        return;
    }

    // ... a jeśli jako IPv4 się nie udało, to jako IPv6
    struct sockaddr_in6 *q = (struct sockaddr_in6 *) ssp;
    memset(q, 0, sizeof(*q));
    if (1 == inet_pton(AF_INET6, arg_addr, &(q->sin6_addr))) {
        q->sin6_family = AF_INET6;
        q->sin6_port = htons(port);
        return;
    }

    fprintf(stderr, "%s is not a valid numeric IP address\n", arg_addr);
    exit(EXIT_FAILURE);
}

void copy_from_to(int input, int output) {
    char buf[4096];
    ssize_t bytes_read, bytes_written;

    do {
        bytes_read = read(input, buf, sizeof(buf));
        if (bytes_read < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        char *data = buf;
        size_t data_len = bytes_read;
        while (data_len > 0) {
            bytes_written = write(output, data, data_len);
            if (bytes_written < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            data = data + bytes_written;
            data_len = data_len - bytes_written;
        }
    } while (bytes_read > 0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        goto bad_args;
    }
    if (0 == strcmp(argv[1], "rcv4")) {
        role = receiver;
        mode = ipv4;
    } else if (0 == strcmp(argv[1], "rcv6")) {
        role = receiver;
        mode = ipv6;
    } else if (0 == strcmp(argv[1], "rcv6only")) {
        role = receiver;
        mode = ipv6only;
    } else if (0 == strcmp(argv[1], "snd4")) {
        role = sender;
        mode = ipv4;
    } else if (0 == strcmp(argv[1], "snd6")) {
        role = sender;
        mode = ipv6;
    } else if (0 == strcmp(argv[1], "snd6only")) {
        role = sender;
        mode = ipv6only;
    } else {
        goto bad_args;
    }
    if (argc != ((role == receiver) ? 3 : 4)) {
        goto bad_args;
    }

    // przetwórz argumenty na strukturę sockaddr

    struct sockaddr_storage addr;

    if (role == receiver) {
        make_sockaddr_for_binding(argv[2], &addr);
    } else {   // role == sender
        make_sockaddr_for_connecting(argv[2], argv[3], &addr);
    }

    print_sockaddr("command line arguments specify", &addr);

    // stwórz gniazdko odpowiedniego typu

    int sock;

    if (mode == ipv4) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
    } else {   // mode == ipv6 lub ipv6only
        sock = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if (sock == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
    // tryb "V6 only" trzeba jawnie włączyć
    if (mode == ipv6only) {
        int value = 1;
        if (-1 == setsockopt(sock, IPPROTO_IPV6,
                             IPV6_V6ONLY, &value, sizeof(value))) {
            perror("setsockopt");
            return EXIT_FAILURE;
        }
    }

    // w zależności od roli odbierz/nawiąż połączenie

    if (role == receiver) {

        // dla prostoty w tym przykładzie nie ma typowej pętli serwerowej
        // odbierającej połączenia od wielu kolejnych klientów

        if (-1 == bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
            perror("bind");
            return EXIT_FAILURE;
        }
        if (-1 == listen(sock, 10)) {
            perror("listen");
            return EXIT_FAILURE;
        }

        int snd_sock;

        if (-1 == (snd_sock = accept(sock, NULL, NULL))) {
            perror("accept");
            return EXIT_FAILURE;
        }

        fprintf(stderr, "accepted connection\n");
        print_endpoints(snd_sock);

        copy_from_to(snd_sock, 1);

        if (-1 == close(snd_sock)) {
            perror("close");
            return EXIT_FAILURE;
        }

    } else {   // role == sender

        if (-1 == connect(sock, (struct sockaddr *) &addr, sizeof(addr))) {
            perror("connect");
            return EXIT_FAILURE;
        }

        fprintf(stderr, "connection established\n");
        print_endpoints(sock);

        copy_from_to(0, sock);

    }

    if (-1 == close(sock)) {
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    bad_args:
    fprintf(stderr, "Usage:\n"
                    "  %s rcv4 | rcv6 | rcv6only tcp_port\n"
                    "  %s snd4 | snd6 | snd6only ip_addr tcp_port\n"
                    "port number range: 1-65535\n", argv[0], argv[0]);
    return EXIT_FAILURE;
}
