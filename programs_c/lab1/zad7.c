#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024

void close_fd(int fd);
void copy_source(int src, int dest, char *buff, size_t buff_size);

int main(int argc, char *argv[]) {
    // assure that user passed 2 arguments
    if (argc < 2) {
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
    int fdRead = open(inFile, O_CREAT | O_RDONLY);
    if(fdRead == -1) {
        perror("Failed to create/open a file\n");
        exit(EXIT_FAILURE);
    }
    
    int fdWrite = open(outFile, O_CREAT | O_WRONLY | O_TRUNC, 0622);
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
        perror("close() function error");
        exit(EXIT_FAILURE);
    }
}

void copy_source(int src, int dest, char *buff, size_t buff_size) {
    ssize_t bytes_read;
    // size_t bytes_to_write;
    const char *LF_ptr;
    int line_counter = 0;
    while ((bytes_read = read(src, buff, buff_size)) > 0) {
        if(bytes_read == -1) {
            perror("read() function error");
            exit(EXIT_FAILURE);
        }
        // bytes_to_write = (size_t) bytes_read;
        int start = 0;
        while((LF_ptr = strchr(buff + start, 10)) != NULL){
            if(++line_counter % 2 != 0) {
                printf("%d\n", LF_ptr - buff - start + 1);
                puts(buff + start);
                write(dest, buff + start, LF_ptr - buff - start + 1);
            }
            start = LF_ptr - buff + 1;
        }
    }
}