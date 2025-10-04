#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>


int debugFlag = 0; 
int displayFlag = 0; 

typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;

} state;

typedef struct fun_desc {
    char *name;
    void (*fun)(state*);
}fun_desc;


//0
void ToggleDebugMode(state* s){
    if(debugFlag == 1){
        debugFlag = 0;
        printf("Debug flag now off\n");
    }
    else{
        debugFlag = 1;
        printf("Debug flag now on\n");
    }
}

//2
void SetFIleName(state* s){
    char newfilename[100];
    printf("insert new file name\n");
    scanf("%s",newfilename );
    strcpy(s->file_name, newfilename); 
    if(debugFlag)
       fprintf(stderr,"Debug: file name set to %s\n", s->file_name);
}

//4
void SetUnitSize(state* s){
    int new_size;
    printf("insert new unit size: \n");
    scanf("%d", &new_size);
    if(new_size == 1 || new_size == 2 || new_size == 4){
        if(debugFlag == 1)
            fprintf(stderr,"Debug: set size to %d\n", new_size);
        s->unit_size = new_size;
    }
    else
        printf("invalid value\n");
}

//5
void LoadIntoMemory(state* s){
    if (strcmp("", s->file_name) == 0){
        printf("Error: empty file name");
        exit(1);
    }
    FILE* file = fopen(s->file_name, "r+");
    if(file == NULL){
        printf("Error: unable to open file");
        exit(1);
    }
    printf("Please enter <location> <length>: \n");
    int location = 0;
    int length = 0;
    scanf("%x %d", &location, &length);
    if(debugFlag == 1){
        fprintf(stderr,"file name: %s\n",s-> file_name);
        fprintf(stderr,"location: %d\n",location);
        fprintf(stderr,"length: %d\n",length);
    }
    fseek(file, location, SEEK_SET);
    s->mem_count = s->unit_size * length;
    fread(s->mem_buf, s->unit_size, length, file);
    printf("Loaded %d units into memory\n", length);
    fclose(file);

}
void printHex(void* buff, int u, int count){
    void* end = buff + u * count;
    char* hex_formats[] = {"%hhX\n", "%hX\n", "No sch unit", "%X\n"};
    printf("Hexadecimal\n=======\n");
    while(buff < end){
        int val = *((int*)(buff));
        printf(hex_formats[u-1], val);
        buff += u;
    }
}

void printDec(void* buff, int u, int count){
    void* end = buff + u * count;
    char* dec_formats[] = {"%hhd\n", "%hd\n", "No such unit", "%d\n"};
    printf("Decimal\n=======\n");
    while(buff < end){
        int val = *((int*)(buff));
        printf(dec_formats[u-1], val);
        buff += u;
    }
}

void ToggleDisplayMode(state* s){ 
    if(displayFlag == 0){ //dec
        displayFlag = 1;
        printf("Display flag now on, hexadecimal representation\n");
    }else{
        displayFlag = 0;
        printf("Display flag now off, decimal representation\n");
    }
}

void MemoryDisplay(state* s){
    int u = 0;
    int address = 0;
    printf("Enter address and length :\n");
    fscanf(stdin, "%x %d", &address, &u);
    if(address==0){
        if(displayFlag==0) printDec(&(s->mem_buf), s->unit_size, u);
        else printHex(&(s->mem_buf), s->unit_size, u);
    }else{
        if (displayFlag==0)  printDec(&address, s->unit_size, u);
        else  printHex(&address, s->unit_size, u);
    }
}

void SaveIntoFile(state* s){
    if (strcmp("", s->file_name) == 0){
        printf("Error: empty file name");
        exit(1);
    }
    FILE* file = fopen(s->file_name, "r+");
    if(file == NULL){
        printf("Error: unable to open file");
        exit(1);
    }
    printf("Please enter <source-address> <target-location> <length> :\n");
    int source_address = 0;
    int length = 0;
    int target_location = 0;
    scanf("%x %x %d", &source_address, &target_location, &length);
    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    if (target_location > file_size) {
        printf("Error, <target-location> is greater than the size of <file_name>\n");
        return;
    }
    fseek(file, 0, SEEK_SET);
    fseek(file, target_location, SEEK_SET);
    if (source_address == 0)
    {
        fwrite(&s->mem_buf, s->unit_size, length, file);
    }
    else{
        fwrite(&source_address, s->unit_size, length, file);
    }
    printf("Wrote %d units into file\n", length);
    fclose(file);
}
void MemoryModify(state* s){
    printf("Please enter <location> <val> : \n");
    int val = 0;
    int location = 0;
    scanf("%x %x", &location, &val);
    if (debugFlag == 1){
        fprintf(stderr,"location: %x, val: %x\n",location, val);
    }
    memcpy(&s->mem_buf[location],&val,s->unit_size);
}


void Quit(state* s){
    if(debugFlag == 1)
        fprintf(stderr,"quitting\n");
    exit(0);
}


fun_desc menu[] = {{"Toggle Debug Mode",ToggleDebugMode},{"Set File Name",SetFIleName},
                {"Set Unit Size",SetUnitSize},{"Load Into Memory",LoadIntoMemory}, {"Toggle display mode",ToggleDisplayMode},
                {"Memory Display", MemoryDisplay},{"Save Into File",SaveIntoFile},
                { "Memory Modify", MemoryModify},{"Quit",Quit},{ NULL,NULL}};


int main(int argc, char **argv){


    state* s = calloc (1 , sizeof(state));
    s->unit_size = 1;
    while (1)
    {
        if(debugFlag == 1){
            fprintf(stderr,"unit_size: %d\n", s->unit_size);
            fprintf(stderr,"file_name: %s\n", s->file_name);
            fprintf(stderr,"mem_count: %d\n", s->mem_count);
        }

        printf("\nChoose action\n");
        for (int i = 0; i < 9; i++)
            printf("%d- %s\n", i, menu[i].name);
        printf("\nUser Choise :\n");

        int option;
        scanf("%d", &option);
        if(option < 0 || option >= 9){
            printf("Not within bounds \n");
        }else{
           printf("Within bounds \n"); 
           menu[option].fun(s); 
        }
    }
}