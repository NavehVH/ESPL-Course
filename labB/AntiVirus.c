#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//virus decription
typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
} link;

// function queries the user for a new signature file name, and sets the signature file name accordingly.
char* SetSigFileName() {
    char* filename = (char*)malloc(256 * sizeof(char));
    if (filename == NULL) {
        perror("Failed to allocate memory");
        exit(1);
    }

    printf("Enter signature file name (default is 'signatures-L'): ");
    if (fgets(filename, 256, stdin) == NULL) {
        perror("Error reading filename");
        exit(1);
    }

    // Remove newline character from the end if exists
    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
        filename[len - 1] = '\0';
    }

    // Set default file name if input is empty
    if (strlen(filename) == 0) {
        strcpy(filename, "signatures-L");
    }

    return filename;
}

//this function receives a file pointer and returns a virus* that represents the next virus in the file.
virus* readVirus(FILE* file) {
    virus* v = (virus*)malloc(sizeof(virus));
    if (v == NULL) {
        perror("Failed to allocate memory");
        exit(1);
    }

    // Read the first 18 bytes (SigSize and virusName)
    if (fread(v, 1, 18, file) != 18) {
        free(v);
        return NULL;
    }

    // Allocate memory for the signature
    v->sig = (unsigned char*)malloc(v->SigSize);
    if (v->sig == NULL) {
        perror("Failed to allocate memory for signature");
        free(v);
        exit(1);
    }

    // Read the signature
    if (fread(v->sig, 1, v->SigSize, file) != v->SigSize) {
        free(v->sig);
        free(v);
        return NULL;
    }

    return v;
}

// this function receives a pointer to a virus structure. The function prints the virus data to stdout.
// It prints the virus name (in ASCII), the virus signature length (in decimal), and the virus signature (in hexadecimal representation).
void printVirus(virus* v) {
    printf("Virus name: %s\n", v->virusName);
    printf("Virus signature length: %d\n", v->SigSize);
    printf("Virus signature:\n");

    for (int i = 0; i < v->SigSize; i++) {
        printf("%02X ", v->sig[i]);
    }

    printf("\n");
}

//Print the data of every link in list to the given stream. Each item followed by a newline character
void list_print(link *virus_list, FILE* output) {
    while (virus_list != NULL) {
        printVirus(virus_list->vir);
        fprintf(output, "\n");
        virus_list = virus_list->nextVirus;
    }
}

//Add a new link with the given data at the beginning of the list and return a pointer to the list
link* list_append(link* virus_list, virus* data) {
    link* new_link = (link*)malloc(sizeof(link));
    if (new_link == NULL) {
        perror("Failed to allocate memory for new link");
        exit(1);
    }
    new_link->vir = data;
    new_link->nextVirus = virus_list;
    return new_link;
}

///* Free the memory allocated by the list. */
void list_free(link *virus_list) {
    while (virus_list != NULL) {
        link* temp = virus_list;
        virus_list = virus_list->nextVirus;
        free(temp->vir->sig);
        free(temp->vir);
        free(temp);
    }
}

//from lab 1
struct fun_desc {
    char *name;
    void (*fun)();
};

char* signatureFileName = NULL;
link* virusList = NULL;

void menu_set_file_name() {
    if (signatureFileName != NULL) {
        free(signatureFileName);
    }
    signatureFileName = SetSigFileName();
}

void menu_load_signatures() {
    if (signatureFileName == NULL) {
        printf("Set signature file name first.\n");
        return;
    }

    FILE* file = fopen(signatureFileName, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char magic[5] = {0};
    if (fread(magic, 1, 4, file) != 4 || (strcmp(magic, "VIRL") != 0 && strcmp(magic, "VIRB") != 0)) {
        fprintf(stderr, "Invalid magic number\n");
        fclose(file);
        return;
    }

    list_free(virusList);
    virusList = NULL;

    virus* v;
    while ((v = readVirus(file)) != NULL) {
        virusList = list_append(virusList, v);
    }

    fclose(file);
}

void menu_print_signatures() {
    if (virusList == NULL) {
        printf("No signatures loaded.\n");
        return;
    }
    list_print(virusList, stdout);
}

// compares the content of the buffer byte-by-byte with the virus signatures stored in the virus_list linked list
void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    for (unsigned int i = 0; i < size; i++) {
        link* current = virus_list;
        while (current != NULL) {
            virus* v = current->vir;
            if (i + v->SigSize <= size && memcmp(buffer + i, v->sig, v->SigSize) == 0) {
                printf("Virus detected!\n");
                printf("Starting byte location: %u\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus signature size: %u\n", v->SigSize);
            }
            current = current->nextVirus;
        }
    }
}

void menu_detect_viruses() {
    char filename[256];
    printf("Enter the name of the file to scan: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        perror("Error reading filename");
        return;
    }

    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
        filename[len - 1] = '\0';
    }

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char buffer[10000];
    size_t file_size = fread(buffer, 1, sizeof(buffer), file);
    if (file_size > 0) {
        detect_virus(buffer, file_size, virusList);
    } else {
        printf("Failed to read file or file is empty.\n");
    }

    fclose(file);
}

void neutralize_virus(char *fileName, int signatureOffset) {
    FILE* file = fopen(fileName, "rb+");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // Set the file position to the offset of the virus signature
    if (fseek(file, signatureOffset, SEEK_SET) != 0) {
        perror("Failed to seek in file");
        fclose(file);
        return;
    }

    // Write the RET (0xC3) instruction to neutralize the virus
    unsigned char ret = 0xC3;
    if (fwrite(&ret, 1, 1, file) != 1) {
        perror("Failed to write to file");
    } else {
        printf("Virus at offset %d neutralized.\n", signatureOffset);
    }

    fclose(file);
}

void menu_fix_file() {
    char filename[256];
    printf("Enter the name of the file to fix: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        perror("Error reading filename");
        return;
    }

    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
        filename[len - 1] = '\0';
    }

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char buffer[10000];
    size_t file_size = fread(buffer, 1, sizeof(buffer), file);
    if (file_size > 0) {
        for (unsigned int i = 0; i < file_size; i++) {
            link* current = virusList;
            while (current != NULL) {
                virus* v = current->vir;
                if (i + v->SigSize <= file_size && memcmp(buffer + i, v->sig, v->SigSize) == 0) {
                    neutralize_virus(filename, i);
                }
                current = current->nextVirus;
            }
        }
    } else {
        printf("Failed to read file or file is empty.\n");
    }

    fclose(file);
}

void menu_quit() {
    list_free(virusList);
    free(signatureFileName);
    exit(0);
}

int main() {
    struct fun_desc menu[] = {
        {"Set signatures file name", menu_set_file_name},
        {"Load signatures", menu_load_signatures},
        {"Print signatures", menu_print_signatures},
        {"Detect viruses", menu_detect_viruses},
        {"Fix file", menu_fix_file},
        {"Quit", menu_quit},
        {NULL, NULL}
    };

    while (1) {
        printf("Choose an option:\n");
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d) %s\n", i, menu[i].name);
        }

        int choice;
        printf("Option: ");
        if (scanf("%d", &choice) == 1) {
            getchar(); // Consume newline character
            if (choice >= 0 && choice < (sizeof(menu) / sizeof(menu[0]) - 1)) {
                menu[choice].fun();
            } else {
                printf("Invalid choice\n");
            }
        } else {
            printf("Invalid input\n");
            while (getchar() != '\n'); // Clear input buffer
        }
    }

    return 0;
}
