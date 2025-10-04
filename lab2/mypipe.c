#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128

//input stream, that stream feeds other stream directly. useful to communicate between processes

int main() {
    int pipe_fd[2];  // Array to hold the read and write file descriptors for the pipe
    pid_t pid;
    char buffer[BUFFER_SIZE];
    char *message = "hello";

    // Create
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipe_fd[0]);  // Close the pipe

        // Write the message
        if (write(pipe_fd[1], message, strlen(message) + 1) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[1]);  // Close the pipe
    } else {
        // Parent process
        close(pipe_fd[1]);  // Close the pipe

        //read message
        if (read(pipe_fd[0], buffer, BUFFER_SIZE) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Print
        printf("Received message: %s\n", buffer);

        close(pipe_fd[0]);  // Close 

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}