#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "LineParser.h"

// maximum size of input
#define MAX_INPUT_SIZE 2048

void execute(cmdLine *pCmdLine) {
    if (pCmdLine->inputRedirect != NULL) {
        int fd_in = open(pCmdLine->inputRedirect, O_RDONLY);
        if (fd_in == -1) {
            perror("Input redirection failed");
            _exit(1);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (pCmdLine->outputRedirect != NULL) {
        int fd_out = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out == -1) {
            perror("Output redirection failed");
            _exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
        perror("Execution failed");
        _exit(1);
    }
}

void changeDirectory(cmdLine *pCmdLine) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "cd: missing argument\n");
        return;
    }

    if (chdir(pCmdLine->arguments[1]) == -1) {
        perror("cd failed");
    }
}

void alarmProcess(cmdLine *pCmdLine) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "alarm: missing argument\n");
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);

    if (kill(pid, SIGCONT) == -1) {
        perror("alarm failed");
    }
}

void blastProcess(cmdLine *pCmdLine) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "blast: missing argument\n");
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);

    if (kill(pid, SIGKILL) == -1) {
        perror("blast failed");
    }
}

void executePipeline(cmdLine *leftCmd, cmdLine *rightCmd) {
    int pipefd[2];
    pid_t cpid1, cpid2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid1 = fork();
    if (cpid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid1 == 0) {
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[1]);
        close(pipefd[0]);

        if (leftCmd->outputRedirect != NULL) {
            fprintf(stderr, "Error: Output redirection not allowed for left-hand side of pipe\n");
            _exit(1);
        }

        execute(leftCmd);
    } else {
        close(pipefd[1]);

        cpid2 = fork();
        if (cpid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (cpid2 == 0) {
            close(STDIN_FILENO);
            dup(pipefd[0]);
            close(pipefd[0]);

            if (rightCmd->inputRedirect != NULL) {
                fprintf(stderr, "Error: Input redirection not allowed for right-hand side of pipe\n");
                _exit(1);
            }

            execute(rightCmd);
        } else {
            close(pipefd[0]);
            waitpid(cpid1, NULL, 0);
            waitpid(cpid2, NULL, 0);
        }
    }
}

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_SIZE];
    char cwd[PATH_MAX];
    cmdLine *parsedLine;
    int debug = 0;

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    }

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            perror("fgets error");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        parsedLine = parseCmdLines(input);
        if (parsedLine == NULL) {
            continue;
        }

        if (strcmp(parsedLine->arguments[0], "quit") == 0) {
            freeCmdLines(parsedLine);
            break;
        }

        if (strcmp(parsedLine->arguments[0], "cd") == 0) {
            changeDirectory(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        if (strcmp(parsedLine->arguments[0], "alarm") == 0) {
            alarmProcess(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        if (strcmp(parsedLine->arguments[0], "blast") == 0) {
            blastProcess(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        if (parsedLine->next != NULL) {
            executePipeline(parsedLine, parsedLine->next);
        } else {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork failed");
                freeCmdLines(parsedLine);
                continue;
            }

            if (pid == 0) {
                execute(parsedLine);
            } else {
                if (debug) {
                    fprintf(stderr, "PID: %d\n", pid);
                    fprintf(stderr, "Executing command: %s\n", parsedLine->arguments[0]);
                }
                if (parsedLine->blocking) {
                    waitpid(pid, NULL, 0);
                }
            }
        }

        freeCmdLines(parsedLine);
    }

    return 0;
}
