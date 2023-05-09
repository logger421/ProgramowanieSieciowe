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

const bool KEEP_HANDLING_REQUESTS = true;
const int MAX_BUFF = 1024;

// creates tcp ipv4 listening socket
int listening_socket_tcp_ipv4(in_port_t port);

int validate_calculate(char *buff, size_t bytes);

ssize_t read_calculate_write(int sock);

void *thread_service(void *arg);

void thread_loop(int srv_sock);

int main(void) {
    int srv_port = 2020;
    int srv_sock;
    // for fun, to try function pointers.
    void (*main_loop)(int) = &thread_loop;

    if ((srv_sock = listening_socket_tcp_ipv4(srv_port)) == -1) {
        return -1;
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

int listening_socket_tcp_ipv4(in_port_t port) {
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
    char *token;
    char *saveptr;
    int written = 0;
    token = strtok_r(buff, "\r\n", &saveptr);

    while (token != NULL) {
        int prev_action = 1;
        int64_t current = 0;
        int64_t part_of_result = 0;

        for (int i = 0; i < strlen(token); i++) {
            if (strchr(allowedCharacters, token[i]) == NULL) {
                memcpy(buff + written, "ERROR\r\n", 7);
                written += 7;
                goto next_token;
            }

            if (isdigit(token[i])) {
                current = current * 10 + (token[i] - '0');
            } else if (token[i] == '+') {
                part_of_result += prev_action * current;

                // check overflow after addition.
                if (part_of_result > INT32_MAX) {
                    memcpy(buff + written, "ERROR\r\n", 7);
                    written += 7;
                    goto next_token;
                }
                prev_action = 1;
                current = 0;
            } else if (token[i] == '-') {
                part_of_result += prev_action * current;

                // check underflow after subtract.
                if (part_of_result < INT32_MIN) {
                    memcpy(buff + written, "ERROR\r\n", 7);
                    written += 7;
                    goto next_token;
                }
                prev_action = -1;
                current = 0;
            }
        }
        part_of_result += prev_action * current;
        prev_action = 1;
        current = 0;
        if ((part_of_result > INT32_MAX) || (part_of_result < INT32_MIN)) {
            memcpy(buff + written, "ERROR\r\n", 7);
            written += 7;
            goto next_token;
        }
        printf("Calc part of result=%d\n", (int32_t) part_of_result);
        if ((written += sprintf(buff + written, "%d\r\n", (int32_t) part_of_result)) == -1) {
            perror("sprintf");
            return -2;
        }

        next_token:
        token = strtok_r(NULL, "\r\n", &saveptr);
    }

    return written;
}

ssize_t read_calculate_write(int sock) {
    char buff[MAX_BUFF];

    ssize_t bytes_read;
    if ((bytes_read = read(sock, buff, MAX_BUFF)) < 0) {
        return -1;
    }

    printf("Data received:%s", buff);
    int to_write = validate_calculate(buff, bytes_read);

    char *to_send = buff;
    ssize_t bytes_written;

    if (to_write == -2) {
        memset(buff, 0, MAX_BUFF);
        return -2;
    } else {
        while ((bytes_written = write(sock, buff, to_write)) > 0) {
            to_send = to_send + bytes_written;
            to_write = to_write - bytes_written;
        }
        if (bytes_written < 0) {
            perror("write");
            return -1;
        }
        memset(buff, 0, MAX_BUFF);

        return bytes_read;
    }
}

void *thread_service(void *arg) {
    int client_socket = *((int *) arg);

    printf("Serving socket=%d\n", client_socket);
    while (read_calculate_write(client_socket) > 0) { ;
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
