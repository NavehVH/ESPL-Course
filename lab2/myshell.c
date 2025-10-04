//0a: 
//Q: Although you loop infinitely, the execution ends after execv. Why is that?
//A: it replaces the current process image with a new process image, the current process is entirely replaced by the new program.

//Q: You must place the full path of an executable file in-order to run properly. For instance: "ls" won't work, whereas "/bin/ls" runs properly. (Why?)
//A: execv does not search for the executable in the directories listed in the PATH. It expects an absolute or relative path to the executable file.

//Q: Wildcards, as in "ls *", are not working. (Again, why?)
//A: does not perform wildcard expansion. It expects the arguments to be fully specified and expanded before it is called.

//Q: execvp fails, so we use _exit(), why?
//A: we don't want the child process to continue executing any further code.

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

// replace current process and pass input\output
void execute(cmdLine *pCmdLine) {
    // input redirection
    if (pCmdLine->inputRedirect != NULL) { //open file standard input
        int fd_in = open(pCmdLine->inputRedirect, O_RDONLY);
        if (fd_in == -1) {
            perror("Input redirection failed");
            _exit(1);
        }
        dup2(fd_in, STDIN_FILENO); //effectively redirecting standard input to read from file
        close(fd_in);
    }

    // output redirection
    if (pCmdLine->outputRedirect != NULL) { //open file standard output
        int fd_out = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666); //used chatgpt
        if (fd_out == -1) {
            perror("Output redirection failed");
            _exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    // Execute the command
    if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) { //replace current process image with new process image
        perror("Execution failed");
        _exit(1); // Exit abnormally if execvp fails
    }
}

//handle cd command and chdir to change the directory
void changeDirectory(cmdLine *pCmdLine) {
    // Check if the cd command has an argument
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "cd: missing argument\n");
        return;
    }

    // Change the current working directory using chdir
    if (chdir(pCmdLine->arguments[1]) == -1) {
        perror("cd failed");
    }
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


/*
    We use fork to create a child process that executes the given command while keeping the shell process (the parent) alive
     to accept and process more commands.
*/
        // Fork a new process (duplicating the calling process)
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
            // Parent process
            if (debug) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Executing command: %s\n", parsedLine->arguments[0]);
            }
            // Parent process, return the process id of the child process to the parent process
            // Wait for the child process to finish if the command is blocking, used chatgpt
            if (parsedLine->blocking) {
                waitpid(pid, NULL, 0);
            }
        }

        // Free the allocated memory for the parsed command line
        freeCmdLines(parsedLine);
    }

    return 0;
}