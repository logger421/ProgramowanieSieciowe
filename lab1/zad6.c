#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 50

void close_fd(int fd);
void copy_source(int src, int dest, char *buff, size_t buff_size);

int main(int argc, char *argv[]) {
    // assure that user passed 2 arguments
    if (argc < 3) {
        printf("Usage: %s <inFile.txt> <outFile.txt>\n", argv[0]);
        return 1;
    }

    char *inFile = argv[1];
    char *outFile = argv[2];
    // assure that params names are valid file names
    if(inFile[0] == '\0' || outFile[0] == '\0') {
        printf("%s invalid argument <[in/out]File.txt>\n", argv[0]);
        return 1;
    }

    char buff[BUFFER_SIZE];
    int fdRead = open(inFile, O_RDONLY);
    if(fdRead == -1) {
        perror("Failed to create/open a file\n");
        exit(EXIT_FAILURE);
    }
    
    int fdWrite = open(outFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(fdWrite == -1) {
        perror("Failed to create/open a file\n");
        exit(EXIT_FAILURE);
    }

    copy_source(fdRead, fdWrite, buff, BUFFER_SIZE);
    close_fd(fdRead);
    close_fd(fdWrite);

    return 0;
}

void close_fd(int fd) {
    if(close(fd) == -1) {
        perror("Something went wrong while closing file");
        exit(EXIT_FAILURE);
    }
}

void copy_source(int src, int dest, char *buff, size_t buff_size) {
    ssize_t bytes_read;
    size_t bytes_to_write;
    ssize_t rc;
    while ((bytes_read = read(src, buff, buff_size)) > 0) {
        if(bytes_read == -1) {
            perror("read() function error");
            exit(EXIT_FAILURE);
        }
        bytes_to_write = (size_t) bytes_read;
        rc = write(dest, buff, bytes_to_write);
        if(rc == -1) {
            perror("write() function error");
            exit(EXIT_FAILURE);
        }
        if(rc < bytes_to_write) {
            puts("Written less data than read, out of memory");
            return 137;
        }
    }
}