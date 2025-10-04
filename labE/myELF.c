#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

// Maximum number of ELF files that can be handled
#define MAX_ELF_FILES 2

// Struct to define menu options and their corresponding functions
typedef struct {
    char *name;
    void (*func)();
} MenuOption;

// Function declarations
void toggleDebugMode();
void examineELFFile();
void printSectionNames();
void printSymbols();
void checkFilesForMerge();
void mergeELFFiles();
void quit();

// Global variables
int debugMode = 0;                      // Debug mode flag
int elfFileCount = 0;                   // Number of ELF files currently loaded
int elfFileDescriptors[MAX_ELF_FILES];  // File descriptors for the ELF files
void *mapStart[MAX_ELF_FILES];          // Mapped memory start addresses for the ELF files
size_t fileSize[MAX_ELF_FILES];         // Sizes of the ELF files

// Function to toggle debug mode on and off
void toggleDebugMode() {
    debugMode = !debugMode;
    printf("Debug mode %s\n", debugMode ? "on" : "off");
}

// Menu options array
MenuOption menu[] = {
    {"Toggle Debug Mode", toggleDebugMode},
    {"Examine ELF File", examineELFFile},
    {"Print Section Names", printSectionNames},
    {"Print Symbols", printSymbols},
    {"Check Files for Merge", checkFilesForMerge},
    {"Merge ELF Files", mergeELFFiles},
    {"Quit", quit},
    {NULL, NULL}
};

// Function to examine an ELF file and print its header information
void examineELFFile() {
    if (elfFileCount >= MAX_ELF_FILES) {
        printf("Cannot handle more than %d ELF files\n", MAX_ELF_FILES);
        return;
    }

    char fileName[256];
    printf("Enter ELF file name: ");
    scanf("%255s", fileName); // Limit input to avoid buffer overflow

    // Open the ELF file
    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("Error getting file size");
        close(fd);
        return;
    }

    // Map the file into memory
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return;
    }

    // Validate ELF header
    Elf32_Ehdr *header = (Elf32_Ehdr *)map;
    if (memcmp(header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("File is not an ELF file\n");
        munmap(map, st.st_size);
        close(fd);
        return;
    }

    // Print ELF header information
    printf("Magic: %c%c%c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    printf("Data: %s\n", header->e_ident[EI_DATA] == ELFDATA2LSB ? "Little Endian" : "Big Endian");
    printf("Entry point: 0x%x\n", header->e_entry);
    printf("Section header offset: %d\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of each section header: %d\n", header->e_shentsize);
    printf("Program header offset: %d\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of each program header: %d\n", header->e_phentsize);

    // Print debug information if debug mode is enabled
    if (debugMode) {
        printf("Debug: ELF file %s mapped at %p with size %lu\n", fileName, map, st.st_size);
    }

    // Store file information
    elfFileDescriptors[elfFileCount] = fd;
    mapStart[elfFileCount] = map;
    fileSize[elfFileCount] = st.st_size;
    elfFileCount++;
}


// Function to print section names of the loaded ELF files
void printSectionNames() {
    if (elfFileCount == 0) {
        printf("No ELF file loaded.\n");
        return;
    }

    const char *section_type_names[] = {
        "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE",
        "NOBITS", "REL", "SHLIB", "DYNSYM", "INIT_ARRAY", "FINI_ARRAY", "PREINIT_ARRAY", "GROUP",
        "SYMTAB_SHNDX", "NUM", "LOOS", "GNU_ATTRIBUTES", "GNU_HASH", "GNU_LIBLIST", "CHECKSUM",
        "LOSUNW", "SUNW_move", "SUNW_COMDAT", "SUNW_syminfo", "GNU_verdef", "GNU_verneed", "GNU_versym"
    };

    for (int fileIndex = 0; fileIndex < elfFileCount; fileIndex++) {
        fileIndex == 0 ? printf("File 1 sections\n") : printf("File 2 sections\n");
        Elf32_Ehdr *header = (Elf32_Ehdr *)mapStart[fileIndex];
        Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)(mapStart[fileIndex] + header->e_shoff);
        const char *stringTable = mapStart[fileIndex] + sectionHeaders[header->e_shstrndx].sh_offset;

        for (int i = 0; i < header->e_shnum; i++) {
            const char *sectionName = stringTable + sectionHeaders[i].sh_name;
            printf("[%2d] %s %08x %08x %08x %s\n",
                   i,
                   sectionName,
                   sectionHeaders[i].sh_addr,
                   sectionHeaders[i].sh_offset,
                   sectionHeaders[i].sh_size,
                   sectionHeaders[i].sh_type < sizeof(section_type_names) / sizeof(section_type_names[0]) ?
                   section_type_names[sectionHeaders[i].sh_type] : "UNKNOWN");
        }
        printf("\n");
    }
}

// Function to print symbols of the loaded ELF files
void printSymbols() {
    if (elfFileCount == 0) {
        printf("No ELF file loaded.\n");
        return;
    }

    const char *fileLabels[] = {"File 1 symbols", "File 2 symbols"};

    for (int fileIndex = 0; fileIndex < elfFileCount; fileIndex++) {
        printf("%s\n", fileLabels[fileIndex]);

        Elf32_Ehdr *header = (Elf32_Ehdr *)mapStart[fileIndex];
        Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)(mapStart[fileIndex] + header->e_shoff);
        const char *sectionNames = mapStart[fileIndex] + sectionHeaders[header->e_shstrndx].sh_offset;

        // Process each section header to find symbol tables
        for (int i = 0; i < header->e_shnum; i++) {
            if (sectionHeaders[i].sh_type == SHT_SYMTAB) {
                Elf32_Sym *symbolTable = (Elf32_Sym *)(mapStart[fileIndex] + sectionHeaders[i].sh_offset);
                int symbolCount = sectionHeaders[i].sh_size / sizeof(Elf32_Sym);
                const char *strtab = mapStart[fileIndex] + sectionHeaders[sectionHeaders[i].sh_link].sh_offset;

                // Print each symbol in the symbol table
                for (int j = 0; j < symbolCount; j++) {
                    const char *symbolName = strtab + symbolTable[j].st_name;
                    const char *sectionName = (symbolTable[j].st_shndx < header->e_shnum) ?
                        sectionNames + sectionHeaders[symbolTable[j].st_shndx].sh_name : "UND";

                    printf("[%2d] %08x %d %s %s\n",
                           j,
                           symbolTable[j].st_value,
                           symbolTable[j].st_shndx,
                           sectionName,
                           symbolName);
                }

                if (debugMode) {
                    printf("Symbol table size: %u\n", sectionHeaders[i].sh_size);
                    printf("Number of symbols: %d\n", symbolCount);
                }
            }
        }
        printf("\n");
    }
}


// Function to check if the loaded ELF files can be merged
void checkFilesForMerge() {
    if (elfFileCount != 2) {
        printf("Error: Exactly two ELF files must be opened and mapped.\n");
        return;
    }

    for (int fileIndex = 0; fileIndex < elfFileCount; fileIndex++) {
        Elf32_Ehdr *header = (Elf32_Ehdr *)mapStart[fileIndex];
        Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)(mapStart[fileIndex] + header->e_shoff);
        int symbolTableCount = 0;

        // Check if each ELF file contains exactly one symbol table
        for (int i = 0; i < header->e_shnum; i++) {
            if (sectionHeaders[i].sh_type == SHT_SYMTAB) {
                symbolTableCount++;
            }
        }

        if (symbolTableCount != 1) {
            printf("Error: Each ELF file must contain exactly one symbol table. Feature not supported.\n");
            return;
        }
    }

    for (int fileIndex = 0; fileIndex < elfFileCount; fileIndex++) {
        Elf32_Ehdr *header1 = (Elf32_Ehdr *)mapStart[fileIndex];
        Elf32_Shdr *sectionHeaders1 = (Elf32_Shdr *)(mapStart[fileIndex] + header1->e_shoff);
        Elf32_Sym *symbolTable1 = NULL;
        int symbolCount1 = 0;
        const char *strtab1 = NULL;

        // Locate the symbol table and string table for the first ELF file
        for (int i = 0; i < header1->e_shnum; i++) {
            if (sectionHeaders1[i].sh_type == SHT_SYMTAB) {
                symbolTable1 = (Elf32_Sym *)(mapStart[fileIndex] + sectionHeaders1[i].sh_offset);
                symbolCount1 = sectionHeaders1[i].sh_size / sizeof(Elf32_Sym);
                strtab1 = (const char *)(mapStart[fileIndex] + sectionHeaders1[sectionHeaders1[i].sh_link].sh_offset);
                break;
            }
        }

        Elf32_Ehdr *header2 = (Elf32_Ehdr *)mapStart[1 - fileIndex];
        Elf32_Shdr *sectionHeaders2 = (Elf32_Shdr *)(mapStart[1 - fileIndex] + header2->e_shoff);
        Elf32_Sym *symbolTable2 = NULL;
        int symbolCount2 = 0;
        const char *strtab2 = NULL;

        // Locate the symbol table and string table for the second ELF file
        for (int i = 0; i < header2->e_shnum; i++) {
            if (sectionHeaders2[i].sh_type == SHT_SYMTAB) {
                symbolTable2 = (Elf32_Sym *)(mapStart[1 - fileIndex] + sectionHeaders2[i].sh_offset);
                symbolCount2 = sectionHeaders2[i].sh_size / sizeof(Elf32_Sym);
                strtab2 = (const char *)(mapStart[1 - fileIndex] + sectionHeaders2[sectionHeaders2[i].sh_link].sh_offset);
                break;
            }
        }

        // Compare symbols from both ELF files
        for (int j = 0; j < symbolCount1; j++) {
            const char *symbolName1 = strtab1 + symbolTable1[j].st_name;
            if (symbolName1 == NULL || strlen(symbolName1) == 0) {
                continue;
            }

            // Check undefined symbols in the first ELF file
            if (symbolTable1[j].st_shndx == SHN_UNDEF) {
                int found = 0;
                for (int k = 0; k < symbolCount2; k++) {
                    const char *symbolName2 = strtab2 + symbolTable2[k].st_name;
                    if (symbolName2 == NULL || strlen(symbolName2) == 0) {
                        continue;
                    }
                    if (strcmp(symbolName1, symbolName2) == 0) {
                        found = 1;
                        if (symbolTable2[k].st_shndx == SHN_UNDEF) {
                            printf("Error: Symbol %s undefined in both ELF files.\n", symbolName1);
                        }
                        break;
                    }
                }
                if (!found) {
                    printf("Error: Symbol %s undefined.\n", symbolName1);
                }
            } else {
                // Check multiply defined symbols
                for (int k = 0; k < symbolCount2; k++) {
                    const char *symbolName2 = strtab2 + symbolTable2[k].st_name;
                    if (symbolName2 == NULL || strlen(symbolName2) == 0) {
                        continue;
                    }
                    if (strcmp(symbolName1, symbolName2) == 0 && symbolTable2[k].st_shndx != SHN_UNDEF) {
                        printf("Error: Symbol %s multiply defined.\n", symbolName1);
                    }
                }
            }
        }
    }
}


// Function to merge the loaded ELF files (not implemented yet)
void mergeELFFiles() {
    printf("Merge functionality not implemented yet.\n");
}

// Function to quit the program, unmap ELF files, and close file descriptors
void quit() {
    for (int i = 0; i < elfFileCount; i++) {
        if (mapStart[i] != MAP_FAILED) {
            munmap(mapStart[i], fileSize[i]);
        }
        if (elfFileDescriptors[i] != -1) {
            close(elfFileDescriptors[i]);
        }
    }
    printf("Quitting...\n");
    exit(0);
}

// Main function to display the menu and handle user input
int main(int argc, char **argv) {
    while (!feof(stdin))  {
        printf("Choose action:\n");
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d-%s\n", i, menu[i].name);
        }

        int choice;
        scanf("%d", &choice);

        if (choice >= 0 && choice < sizeof(menu) / sizeof(MenuOption) - 1) {
            menu[choice].func();
        } else {
            printf("Invalid choice\n");
        }
    }
    return 0;
}
