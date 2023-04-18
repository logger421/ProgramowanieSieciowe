#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

const bool KEEP_HANDLING_REQUESTS = true;
// TODO(WL) Try to consolidate validation & calculating result.
// validate_data();
// calculate_result();

// creates tcp ipv4 listening socket
int listening_socket_tcp_ipv4(in_port_t port);

void thread_loop(int srv_sock);

void *thread_service(void *arg);

ssize_t read_calc_write(int sock);

void validate_calculate(char* buff, int bytes);

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

void validate_calculate(char* buff, int bytes) {

}


ssize_t read_calc_write(int sock) {
    char buf[4096];
    ssize_t bytes = read(sock, buf, sizeof(buf));
    if(bytes < 0) {
        return -1;
    }

    validate_calculate(buf, bytes);

    char* response = buf;

    return 1;
}

void *thread_service(void *arg) {
    int socket = *((int *) arg);

    while (read_calc_write(socket) > 0) {
        ;
    }

    if(close(socket) == -1) {
        perror("close");
        exit(-1);
    }

    free(arg);
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

        int fd;
        if( (fd = accept(srv_sock, (struct sockaddr *) &a, &a_len)) == -1) {
            perror("accept");
            exit(-2);
        }

        int *param = (int *) malloc(sizeof(int));
        if(param == NULL) {
            perror("malloc");
            exit(0);
        }

        *param = fd;
        pthread_t th;
        if((rc = pthread_create(&th, &attr, thread_service, param)) != 0) {
            printf("pthread_create error rc=%d\n", rc);
            goto cleanup_sock;
        }

        continue;

        cleanup_sock:
        if(close(fd) == -1) {
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
