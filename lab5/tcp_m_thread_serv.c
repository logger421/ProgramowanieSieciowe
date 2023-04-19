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
#include <limits.h>
#include <stdint.h>

const bool KEEP_HANDLING_REQUESTS = true;
// TODO(WL) Try to consolidate validation & calculating result.
// validate_data();
// calculate_result();

// creates tcp ipv4 listening socket
int listening_socket_tcp_ipv4(in_port_t port);

void thread_loop(int srv_sock);

void *thread_service(void *arg);

ssize_t read_calculate_write(int sock, int32_t* result, int* prev_action);

void validate_calculate(char *buff, size_t bytes, int32_t* result, int* prev_action);

int main(void) {
    int srv_port = 2020;
    int srv_sock;
    // for fun to try function pointers.
    void (*main_loop)(int) = &thread_loop;

    printf("Listening on port 2020\n");
    if ((srv_sock = listening_socket_tcp_ipv4(srv_port)) == -1) {
        return 1;
    }

    printf("Staring main loop\n");
    main_loop(srv_sock);
    printf("Done with main loop\n");

    if (close(srv_sock) == -1) {
        perror("close");
        return 1;
    }

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

void validate_calculate(char *buff, size_t bytes, int32_t* result, int* prev_action) {
    const char allowedCharacters[] = "0123456789+-";

    int64_t current = 0;

    if (!isdigit(buff[0])) {
        memcpy(buff, "ERROR\r\n", 7);
        return;
    }

    for (int i = 0; i < bytes; i++) {
        if (strchr(allowedCharacters, buff[i]) == NULL) {
            memcpy(buff, "ERROR\r\n", 7);
            break;
        }

        if(buff[i] == '\r' && buff[i+1] == '\n') {
            *prev_action = 1;
        }
    }
}

ssize_t read_calculate_write(int sock, int32_t* result, int* prev_action) {
    char buff[1024];

    ssize_t bytes_read;
    if((bytes_read = read(sock, buff, sizeof(buff))) < 0) {
        return -1;
    }

    validate_calculate(buff, bytes_read, result, prev_action);

    int to_write;
    char* to_send = buff;
    if((to_write = sprintf(to_send, "%d\r\n", *result)) == -1) {
        perror("sprintf");
        return 1;
    }

    ssize_t bytes_written;
    while((bytes_written = write(sock, buff, to_write)) > 0) {
        to_send = to_send + bytes_written;
        to_write = to_write - bytes_written;
    }
    return bytes_read;
}

void *thread_service(void *arg) {
    int socket = *((int *) arg);

    int32_t* result = (int32_t *) malloc(sizeof(int32_t));
    if (result == NULL) {
        printf("No memory for result variable allocation\n");
        return NULL;
    }

    int* prev_action = (int *) malloc(sizeof(int));
    *prev_action = 1;

    while (read_calculate_write(socket, result, prev_action) > 0) {
        ;
    }

    if (close(socket) == -1) {
        perror("close");
        exit(-1);
    }

    free(arg);
    free(result);
    free(prev_action);
    return NULL;
}

void thread_loop(int srv_sock) {
    pthread_attr_t attr;
    int rc;

    rc = pthread_attr_init(&attr);

    if (rc != 0) {
        printf("pthread_attr_init error rc=%d\n", rc);
        return;
    }

    // we want thread to be in detached state bc handling client req could take a lot of time.
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (rc != 0) {
        printf("pthread_attr_setdetachstate error rc=%d\n", rc);
        goto cleanup_attr;
    }

    while (KEEP_HANDLING_REQUESTS) {
        struct sockaddr_in a;
        socklen_t a_len = sizeof(a);
        memset(&a, 0, sizeof(a_len));

        int cli_sock;
        if ((cli_sock = accept(srv_sock, (struct sockaddr *) &a, &a_len)) == -1) {
            perror("accept");
            exit(-2);
        }

        int *param = (int *) malloc(sizeof(int));
        if (param == NULL) {
            perror("malloc");
            exit(0);
        }

        *param = cli_sock;
        pthread_t th;
        if ((rc = pthread_create(&th, &attr, thread_service, param)) != 0) {
            printf("pthread_create error rc=%d\n", rc);
            goto cleanup_sock;
        }

        continue;

        cleanup_sock:
        if (close(cli_sock) == -1) {
            perror("close");
            exit(-1);
        }
    }

    cleanup_attr:
    rc = pthread_attr_destroy(&attr);
    if (rc != 0) {
        printf("pthread_attr_destroy error rc=%d\n", rc);
    }
}
