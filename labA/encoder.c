#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void encode(char* encoding, FILE* infile, FILE* outfile, int mode);

int main(int argc, char *argv[]) {

    int debug_mode = 1;

    // Input and output file pointers
    FILE *infile = stdin;
    FILE *outfile = stdout;

    char *encoding = NULL;
    int mode = -1;

    for (int i = 1; i < argc; i++) {
        // Task 1: Handle debug mode
        if (strcmp(argv[i], "-D") == 0) {
            debug_mode = 0;
            continue; 
        }

        if (strcmp(argv[i], "+D") == 0) {
            debug_mode = 1;
            continue; 
        }

        // Debug output
        if (debug_mode) {
            fprintf(stderr, "Debug: %s\n", argv[i]);
        }

        // Task 2 + 3: Handle encoding/decoding
        if (strncmp(argv[i], "+e", 2) == 0) {
            encoding = argv[i] + 2;
            mode = 0;
            continue;
        }
        if (strncmp(argv[i], "-e", 2) == 0) {
            encoding = argv[i] + 2;
            mode = 1;
            continue;
        }

        // Handle input file
        if (strncmp(argv[i], "-I", 2) == 0) {
            char *input_filename = argv[i] + 2;
            infile = fopen(input_filename, "r");
            if (infile == NULL) {
                fprintf(stderr, "Error: Could not open input file %s\n", input_filename);
                exit(EXIT_FAILURE);
            }
            continue;
        }

        // Handle output file
        if (strncmp(argv[i], "-O", 2) == 0) {
            char *output_filename = argv[i] + 2;
            outfile = fopen(output_filename, "w");
            if (outfile == NULL) {
                fprintf(stderr, "Error: Could not open output file %s\n", output_filename);
                exit(EXIT_FAILURE);
            }
            continue;
        }
    }

    // Process the input and write to the output
    if (encoding != NULL && mode != -1) {
        encode(encoding, infile, outfile, mode);
    } else {
        // If no encoding specified, just copy input to output
        char ch;
        while ((ch = fgetc(infile)) != EOF) {
            fputc(ch, outfile);
        }
    }

    // Closing if open
    if (infile != stdin) fclose(infile);
    if (outfile != stdout) fclose(outfile);

    return 0;
}

void encode(char* encoding, FILE* infile, FILE* outfile, int mode) {
    int j = 0;
    char ch;

    while ((ch = fgetc(infile)) != EOF) {
        int encoding_value = encoding[j] - '0';

        if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
          
            if (mode == 0) {
                ch += encoding_value;
            } else {
                ch -= encoding_value;
            }
            
            if (ch > 'z') {
                ch = 'a' + ((ch - 'a') % 26);
            } else if (ch > '9' && ch < 'a') {
                ch = '0' + ((ch - '0') % 10);
            }

            // Move to the next digit
            j = (j + 1) % strlen(encoding);
        }

        fputc(ch, outfile);
    }
}
