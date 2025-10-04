#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "LineParser.h"

// Maximum size of input
#define MAX_INPUT_SIZE 2048

// Process status constants
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

// Define the process struct
typedef struct process {
    cmdLine* cmd;         // The parsed command line
    pid_t pid;            // The process ID running the command
    int status;           // Status of the process: RUNNING/SUSPENDED/TERMINATED
    struct process *next; // Pointer to the next process in the list
} process;

process* process_list = NULL;

// Function to add a process to the process list
void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process *newProcess = (process*)malloc(sizeof(process));
    newProcess->cmd = cmd;
    newProcess->pid = pid;
    newProcess->status = RUNNING; // Initially, the process is running
    newProcess->next = *process_list;
    *process_list = newProcess;
}

// Function to update the status of a process in the list
void updateProcessStatus(process* process_list, int pid, int status) {
    process* current = process_list;
    while (current != NULL) {
        if (current->pid == pid) {
            current->status = status;
            break;
        }
        current = current->next;
    }
}

// Function to update the process list
void updateProcessList(process **process_list) {
    process* current = *process_list;
    process* prev = NULL;
    int status;
    pid_t result;

    while (current != NULL) {
        result = waitpid(current->pid, &status, WNOHANG);
        if (result == 0) {
            // The process is still running or suspended
            current = current->next;
        } else if (result == -1) {
            // Error occurred
            perror("waitpid failed");
            current = current->next;
        } else {
            // The process has changed state
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                current->status = TERMINATED;
            } else if (WIFSTOPPED(status)) {
                current->status = SUSPENDED;
            } else if (WIFCONTINUED(status)) {
                current->status = RUNNING;
            }
            // Remove the process from the list if it is terminated
            if (current->status == TERMINATED) {
                if (prev == NULL) {
                    *process_list = current->next;
                } else {
                    prev->next = current->next;
                }
                process* temp = current;
                current = current->next;
                freeCmdLines(temp->cmd);
                free(temp);
            } else {
                prev = current;
                current = current->next;
            }
        }
    }
}

// Function to print the process list
void printProcessList(process** process_list) {
    updateProcessList(process_list);

    process *current = *process_list;
    int index = 0;
    printf("PID         Command     STATUS\n");
    printf("--------------------------------------------\n");
    while (current != NULL) {
        printf("%d       %s    %s\n", current->pid, current->cmd->arguments[0],
            (current->status == RUNNING) ? "Running" :
            (current->status == SUSPENDED) ? "Suspended" : "Terminated");
        current = current->next;
        index++;
    }
}

// Function to free the process list
void freeProcessList(process* process_list) {
    process* current = process_list;
    while (current != NULL) {
        process* temp = current;
        current = current->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }
}

// Function to execute a command
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

// Function to change directory
void changeDirectory(cmdLine *pCmdLine) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "cd: missing argument\n");
        return;
    }

    if (chdir(pCmdLine->arguments[1]) == -1) {
        perror("cd failed");
    }
}

// Function to handle the "procs" command
void handleProcs() {
    printProcessList(&process_list);
}

// Function to send a signal to a process
void signalProcess(cmdLine *pCmdLine, int signal) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "Usage: %s <process id>\n", pCmdLine->arguments[0]);
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);

    if (kill(pid, signal) == -1) {
        perror("kill failed");
    } else {
        switch (signal) {
            case SIGTSTP:
                printf("Sent SIGTSTP to process %d\n", pid);
                updateProcessStatus(process_list, pid, SUSPENDED);
                break;
            case SIGINT:
                printf("Sent SIGINT to process %d\n", pid);
                updateProcessStatus(process_list, pid, TERMINATED);
                break;
            case SIGCONT:
                printf("Sent SIGCONT to process %d\n", pid);
                updateProcessStatus(process_list, pid, RUNNING);
                break;
            default:
                break;
        }
    }
}

// Function to handle the "sleep" command
void handleSleep(cmdLine *pCmdLine) {
    signalProcess(pCmdLine, SIGTSTP);
}

// Function to handle the "blast" command
void handleBlast(cmdLine *pCmdLine) {
    signalProcess(pCmdLine, SIGINT);
}

// Function to handle the "alarm" command
void handleAlarm(cmdLine *pCmdLine) {
    signalProcess(pCmdLine, SIGCONT);
}

void executePipe(cmdLine *leftCmd, cmdLine *rightCmd) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork failed");
        return;
    }

    if (pid1 == 0) { // First child process
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[1]);
        close(pipefd[0]);
        execute(leftCmd);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork failed");
        return;
    }

    if (pid2 == 0) { // Second child process
        close(STDIN_FILENO);
        dup(pipefd[0]);
        close(pipefd[0]);
        close(pipefd[1]);
        execute(rightCmd);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_SIZE];
    char cwd[PATH_MAX];
    cmdLine *parsedLine;

    int debug = 0;

    // Check for the debug flag
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    }

    while (1) { //loop to read commands
        // Display the current working directory as a prompt
        if (getcwd(cwd, sizeof(cwd)) != NULL) { //working directory, store it in the buffer cwd
            printf("%s> ", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }

        // Read input from the user
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            perror("fgets error");
            break; //not sure its break or continue
        }

        // Remove the newline character from the end of the input
        input[strcspn(input, "\n")] = 0;

        parsedLine = parseCmdLines(input); // Parse the input using parseCmdLines
        if (parsedLine == NULL) {
            continue;
        }

        // Check for built-in commands

        // "exit" command
        if (strcmp(parsedLine->arguments[0], "exit") == 0) {
            freeProcessList(process_list);
            freeCmdLines(parsedLine);
            break;
        }

        // "cd" command
        if (strcmp(parsedLine->arguments[0], "cd") == 0) {
            changeDirectory(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        // "procs" command
        if (strcmp(parsedLine->arguments[0], "procs") == 0) {
            handleProcs();
            freeCmdLines(parsedLine);
            continue;
        }

        // "alarm" command
        if (strcmp(parsedLine->arguments[0], "alarm") == 0) {
            handleAlarm(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        // "blast" command
        if (strcmp(parsedLine->arguments[0], "blast") == 0) {
            handleBlast(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        // "sleep" command
        if (strcmp(parsedLine->arguments[0], "sleep") == 0) {
            handleSleep(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        if (parsedLine->next != NULL) { // Check if there's a pipe
            executePipe(parsedLine, parsedLine->next);
            freeCmdLines(parsedLine->next);
        } else {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork failed");
                freeCmdLines(parsedLine);
                continue;
            }

            if (pid == 0) {
                // Returned as a child process
                execute(parsedLine);
            } else {
                addProcess(&process_list, parsedLine, pid);
                if (debug) {
                    fprintf(stderr, "PID: %d\n", pid);
                    fprintf(stderr, "Executing command: %s\n", parsedLine->arguments[0]);
                }
                // Parent process, return the process id of the child process to the parent process
                // Wait for the child process to finish if the command is blocking
                if (parsedLine->blocking) {
                    waitpid(pid, NULL, 0);
                    updateProcessStatus(process_list, pid, TERMINATED); // Update the status to terminated
                }
            }
        }
    }

    // Free the process list before exiting
    freeProcessList(process_list);

    return 0;
}
