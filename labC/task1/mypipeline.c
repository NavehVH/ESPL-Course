#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t cpid1, cpid2;

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>forking…)\n");

    // Fork the first child process
    cpid1 = fork();
    if (cpid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid1 == 0) {
        // First child process
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");

        close(STDOUT_FILENO);           // Close the standard output
        dup(pipefd[1]);                 // Duplicate the write-end of the pipe to standard output
        close(pipefd[1]);               // Close the original write-end of the pipe
        close(pipefd[0]);               // Close the read-end of the pipe in this process

        fprintf(stderr, "(child1>going to execute cmd: ls -l …)\n");

        // Execute "ls -l"
        execlp("ls", "ls", "-l", (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid1);
    }

    fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
    close(pipefd[1]); // Close the write end of the pipe in the parent

    fprintf(stderr, "(parent_process>forking…)\n");

    // Fork the second child process
    cpid2 = fork();
    if (cpid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid2 == 0) {
        // Second child process
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");

        close(STDIN_FILENO);            // Close the standard input
        dup(pipefd[0]);                 // Duplicate the read-end of the pipe to standard input
        close(pipefd[0]);               // Close the original read-end of the pipe

        fprintf(stderr, "(child2>going to execute cmd: tail -n 2 …)\n");

        // Execute "tail -n 2"
        execlp("tail", "tail", "-n", "2", (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid2);
    }

    fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
    close(pipefd[0]); // Close the read end of the pipe in the parent

    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");

    // Wait for both child processes to terminate
    waitpid(cpid1, NULL, 0);
    waitpid(cpid2, NULL, 0);

    fprintf(stderr, "(parent_process>exiting…)\n");

    return 0;
}