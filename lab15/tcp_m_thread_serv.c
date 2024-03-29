#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

enum {
    ipv4, ipv6, ipv6only
} mode;

const bool KEEP_HANDLING_REQUESTS = true;
const int MAX_BUFF = 1024;

void print_sockaddr(const char *msg, struct sockaddr_storage *ssp);

void make_sockaddr_for_binding(const int port, struct sockaddr_storage *ssp);

// creates tcp ipv4 listening socket
int listening_socket_tcp(in_port_t port, struct sockaddr_storage *ssp);

int validate_calculate(char *buff, size_t bytes);

ssize_t read_calculate_write(int sock);

void *thread_service(void *arg);

void thread_loop(int srv_sock);

int main(int argc, char *argv[]) {
//    ./a.out [ipv4, ipv6, ipv6only]
    if (argc != 2) {
        puts("Bad args! Should be: ./a.out ipv4 | ipv6 | ipv6only");
        return -1;
    }

    if (0 == strcmp(argv[1], "ipv4")) {
        mode = ipv4;
    } else if (0 == strcmp(argv[1], "ipv6")) {
        mode = ipv6;
    } else if (0 == strcmp(argv[1], "ipv6only")) {
        mode = ipv6only;
    } else {
        puts("Bad args! Should be: ./a.out ipv4 | ipv6 | ipv6only");
        return -1;
    }

    int srv_port = 2020;
    struct sockaddr_storage addr;
    // for fun, to try function pointers.
    void (*main_loop)(int) = &thread_loop;

    make_sockaddr_for_binding(srv_port, &addr);

    print_sockaddr("command line arguments specify", &addr);

    int srv_sock;

    if (mode == ipv4) {
        srv_sock = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        srv_sock = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if (srv_sock == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
    if (mode == ipv6only) {
        int value = 1;
        if (-1 == setsockopt(srv_sock, IPPROTO_IPV6,
                             IPV6_V6ONLY, &value, sizeof(value))) {
            perror("setsockopt");
            return EXIT_FAILURE;
        }
    }

    if (-1 == bind(srv_sock, (struct sockaddr *) &addr, sizeof(addr))) {
        perror("bind");
        return EXIT_FAILURE;
    }
    if (-1 == listen(srv_sock, 10)) {
        perror("listen");
        return EXIT_FAILURE;
    }

    puts("Server started, listening on port: 2020");
    main_loop(srv_sock);

    if (close(srv_sock) == -1) {
        perror("close");
        return -1;
    }
    puts("Server listening socket closed");
    return 0;
}

int listening_socket_tcp(in_port_t port, struct sockaddr_storage *ssp) {
    int fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in a = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = htonl(INADDR_ANY),
            .sin_port = htons(port)
    };

    if (bind(fd, (struct sockaddr *) &a, sizeof(a)) == -1) {
        perror("bind");
        goto close_and_fail;
    }

    if (listen(fd, 10) == -1) {
        perror("listen");
        goto close_and_fail;
    }

    return fd;

    close_and_fail:
    close(fd);
    return -1;
}

int validate_calculate(char *buff, size_t bytes) {
    const char allowedCharacters[] = "0123456789+-\r\n";
    int written = 0;
    int prev_action = 1;
    int64_t current = 0;
    int64_t part_of_result = 0;

    if (!(buff[bytes - 2] == '\r' && buff[bytes - 1] == '\n')) {
        puts("invalid data, \\r\\n sequence missing");
        return -1;
    }

    for (int i = 0; i < bytes; i++) {
        if (strchr(allowedCharacters, buff[i]) == NULL) {
            return -1;
        }
        if (isdigit(buff[i])) {
            current = current * 10 + (buff[i] - '0');
        } else if (buff[i] == '+') {
            part_of_result += prev_action * current;

            // check overflow after addition.
            if (part_of_result > INT32_MAX) {
                puts("Overflow/underflow error");
                return -1;
            }
            prev_action = 1;
            current = 0;
        } else if (buff[i] == '-') {
            part_of_result += prev_action * current;

            // check underflow after subtract.
            if (part_of_result < INT32_MIN) {
                puts("Overflow/underflow error");
                return -1;
            }
            prev_action = -1;
            current = 0;
        }
    }
    part_of_result += prev_action * current;
    if ((part_of_result > INT32_MAX) || (part_of_result < INT32_MIN)) {
        puts("Overflow/underflow error");
        return -1;
    }

    printf("Calc part of result=%d\n", (int32_t) part_of_result);
    if ((written = sprintf(buff, "%d\r\n", (int32_t) part_of_result)) == -1) {
        perror("sprintf");
        return -2;
    }

    return written;
}

ssize_t read_calculate_write(int sock) {
    char read_buff[MAX_BUFF];
    char to_process_buff[MAX_BUFF];
    char output_buff[MAX_BUFF];

    ssize_t bytes_read;
    ssize_t bytes_written;

    int to_write;
    int input_len = 0;

    while ((bytes_read = read(sock, read_buff, MAX_BUFF)) > 0) {
        // TODO(WL) prevent security issue, memory leakage when client only sends new data, never terminating it with "\r\n".
        memcpy(to_process_buff + input_len, read_buff, bytes_read);
        input_len += bytes_read;

        char *input_ptr = to_process_buff;
        int input_left = input_len;

        while (input_left > 0) {
            // find the end of expression (\r\n)
            char *end_ptr = memchr(input_ptr, '\r', input_left);
            if (end_ptr == NULL) {
                // if end not found, wait for more data
                break;
            }

            to_write = validate_calculate(input_ptr, end_ptr - input_ptr + 2);
            if (to_write == -1) {
                memcpy(output_buff, "ERROR\r\n", 7);
                to_write = 7;
            } else {
                to_write = snprintf(output_buff, MAX_BUFF, "%s", input_ptr);
            }

            char *to_send = output_buff;
            while (to_write > 0) {
                bytes_written = write(sock, to_send, to_write);
                if (bytes_written < 0) {
                    perror("write");
                    return -1;
                }
                to_send += bytes_written;
                to_write -= bytes_written;
            }

            // update input buffer and pointer
            // +2 because of existing "\r\n". Found with memchr end_ptr -> at position of '\r'.
            input_left -= (end_ptr - input_ptr + 2);
            input_ptr = end_ptr + 2;
        }

        // shift remaining data to the beginning of the buffer
        if (input_left > 0) {
            memmove(to_process_buff, input_ptr, input_left);
        }
        input_len = input_left;
    }

    if (bytes_read < 0) {
        perror("read");
        return -1;
    }

    return 0;
}

void *thread_service(void *arg) {
    int client_socket = *((int *) arg);

    printf("Serving socket=%d\n", client_socket);
    if (read_calculate_write(client_socket) != 0) {
        puts("read_calculate_write error");
    }

    printf("Closing socket=%d\n", client_socket);
    if (close(client_socket) == -1) {
        perror("close");
        exit(-1);
    }

    // free alocated memory.
    free(arg);
    return NULL;
}

void thread_loop(int srv_sock) {
    pthread_attr_t attr;
    int rc;

    if ((rc = pthread_attr_init(&attr)) != 0) {
        printf("pthread_attr_init error rc=%d\n", rc);
        return;
    }

    // we want thread to be in detached state bc handling client requests could take time.
    if ((rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
        printf("pthread_attr_setdetachstate error rc=%d\n", rc);
        goto cleanup_attr;
    }

    // main loop, responsible for accepting client connections and passing them to threads.
    while (KEEP_HANDLING_REQUESTS) {
        struct sockaddr_in a;
        socklen_t a_len = sizeof(a);
        memset(&a, 0, sizeof(a_len));

        int cli_sock;
        if ((cli_sock = accept(srv_sock, (struct sockaddr *) &a, &a_len)) == -1) {
            perror("Error occurred on accept function, connection refused.");
            continue;
        }
        puts("New client accepted");

        int *param = (int *) malloc(sizeof(int));
        if (param == NULL) {
            perror("Error occurred on malloc, connection refused.");
            goto cleanup_sock;
        }

        *param = cli_sock;
        pthread_t th;
        if ((rc = pthread_create(&th, &attr, thread_service, param)) != 0) {
            printf("pthread_create error rc=%d\n", rc);
            goto cleanup_sock;
        }
        puts("New pthread created");

        continue;

        // if error occurred while creating thread.
        cleanup_sock:
        if (close(cli_sock) == -1) {
            perror("close");
            continue;
        }
    }

    // if error occurred while creating thread attribute.
    cleanup_attr:
    rc = pthread_attr_destroy(&attr);
    if (rc != 0) {
        printf("pthread_attr_destroy error rc=%d\n", rc);
    }
}

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

void make_sockaddr_for_binding(const int port, struct sockaddr_storage *ssp) {

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