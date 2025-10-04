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

// Function to update the status of the processes in the list
void updateProcessList(process** process_list) {
    process* current = *process_list;
    int status;
    pid_t result;

    while (current != NULL) {
        result = waitpid(current->pid, &status, WNOHANG);
        if (result == 0) {
            // The process is still running
            current->status = RUNNING;
        } else if (result == -1) {
            // Error occurred
            perror("waitpid failed");
        } else {
            // The process has terminated
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                current->status = TERMINATED;
            } else if (WIFSTOPPED(status)) {
                current->status = SUSPENDED;
            } else if (WIFCONTINUED(status)) {
                current->status = RUNNING;
            }
        }
        current = current->next;
    }
}

// Function to print the process list
void printProcessList(process** process_list) {
    updateProcessList(process_list);

    process *current = *process_list;
    int index = 0;
    printf("Index | PID       | Status     | Command\n");
    printf("----------------------------------------------------\n");
    while (current != NULL) {
        printf("%d     | %d  | %s   | ", index, current->pid, 
            (current->status == RUNNING) ? "Running" :
            (current->status == SUSPENDED) ? "Suspended" : "Terminated");
        for (int i = 0; i < current->cmd->argCount; i++) {
            printf("%s ", current->cmd->arguments[i]);
        }
        printf("\n");
        current = current->next;
        index++;
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

// wakeup process, SIGCONT signal the process to wake it up
void alarmProcess(cmdLine *pCmdLine) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "alarm: missing argument\n");
        return;
    }

    //process ID arg
    pid_t pid = atoi(pCmdLine->arguments[1]);

    // wake up the process, kill send the SIGCONT
    if (kill(pid, SIGCONT) == -1) {
        perror("alarm failed");
    }
}

// terminate a process, SIGKILL signal to a process to terminate it
void blastProcess(cmdLine *pCmdLine) {
    // Check if the blast command has an argument
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "blast: missing argument\n");
        return;
    }

    // Get the process ID from the argument
    pid_t pid = atoi(pCmdLine->arguments[1]);

    // Send the SIGKILL signal to terminate the process
    if (kill(pid, SIGKILL) == -1) {
        perror("blast failed");
    }
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

        // "quit" command
        if (strcmp(parsedLine->arguments[0], "quit") == 0) {
            freeCmdLines(parsedLine); //breaks the loop and exit the shell
            break;
        }

        // "cd" command
        if (strcmp(parsedLine->arguments[0], "cd") == 0) { //change the directory
            changeDirectory(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        // Check for the "alarm" command
        if (strcmp(parsedLine->arguments[0], "alarm") == 0) { //send sigcont
            alarmProcess(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        // Check for the "blast" command
        if (strcmp(parsedLine->arguments[0], "blast") == 0) { //send sigkill
            blastProcess(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        // Check for the "procs" command
        if (strcmp(parsedLine->arguments[0], "procs") == 0) {
            handleProcs();
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
                // returned as a child process
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
                    process_list->status = TERMINATED; // Update the status to terminated
                }
            }
        }
    }

    return 0;
}
