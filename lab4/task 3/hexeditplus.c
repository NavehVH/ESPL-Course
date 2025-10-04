#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

typedef struct fun_desc {
    char *name;
    void (*fun)(state*);
}fun_desc;

int debug = 0; //off
int display = 0; //dec


void print_menu(fun_desc menu[], int menu_length, state* s){
    if(debug == 1){
        fprintf(stderr,"unit_size: %d\n", s->unit_size);
        fprintf(stderr,"file_name: %s\n", s->file_name);
        fprintf(stderr,"mem_count: %d\n", s->mem_count);
    }

    printf("\nChoose action\n");
    for (int i = 0; i < menu_length; i++)
        printf("%d- %s\n", i, menu[i].name);
    printf("\nOption :\n");
}

void ToggleDebugMode(state* s){
    if(debug == 0){
        debug = 1;
        printf("Debug flag now on\n");
    }
    else{
        debug = 0;
        printf("Debug flag now off\n");
    }
}

void SetFIleName(state* s){
    printf("insert a new file name\n");
    char newfilename[100];
    scanf("%s",newfilename );
    strcpy(s->file_name, newfilename); 
    if(debug)
       fprintf(stderr,"Debug: file name set to %s\n", s->file_name);
}

void SetUnitSize(state* s){
    int new_unit;
    printf("insert new unit size: \n");
    scanf("%d", &new_unit);
    if(new_unit == 1 || new_unit == 2 || new_unit == 4){
        if(debug == 1)
            fprintf(stderr,"Debug: set size to %d\n", new_unit);
        s->unit_size = new_unit;
    }
    else
        printf("invalid value\n");
}

void Quit(state* s){
    if(debug == 1)
        fprintf(stderr,"quitting\n");
    exit(0);
}

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
    int length = 0;
    int location = 0;
    scanf("%x %d", &location, &length);
    if(debug == 1){
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
void printMem(void* buff, int count, int u){
    void* end = buff + u * count;
    char* dec_formats[] = {"%hhd\n", "%hd\n", "No such unit", "%d\n"};
    char* hex_formats[] = {"%hhX\n", "%hX\n", "No sch unit", "%X\n"};
    while(buff < end){
        int val = *((int*)(buff));
        if(display == 0){//dec
            printf(dec_formats[u-1], val);
        }else{
            printf(hex_formats[u-1], val);
        }
        buff += u;
    }
}
void MemoryDisplay(state* s){
    int u = 0;
    int address = 0;
    printf("insert num of address and length (seperated by a sibgle space) :\n");
    fscanf(stdin, "%x %d", &address, &u);

    (display == 0) ? printf("Decimal\n=======\n"): printf("Hexadecimal\n=======\n");
    (address == 0) ? printMem(&(s->mem_buf), u, s->unit_size) : printMem(&address, u, s->unit_size);
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
    if (target_location > ftell(file)){
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
    int location = 0;
    int val = 0;
    scanf("%x %x", &location, &val);
    if (debug == 1){
        fprintf(stderr,"location: %x, val: %x\n",location, val);
    }
    memcpy(&s->mem_buf[location],&val,s->unit_size);
}
void ToggleDisplayMode(state* s){ 
    if(display == 0){ //dec
        display = 1;
        printf("Display flag now on, hexadecimal representation\n");
    }else{
        display = 0;
        printf("Display flag now off, decimal representation\n");
    }
}



int main(int argc, char **argv){

    fun_desc menu[] = {{"Toggle Debug Mode",ToggleDebugMode},{"Set File Name",SetFIleName},
                    {"Set Unit Size",SetUnitSize},{"Load Into Memory",LoadIntoMemory}, {"Toggle display mode",ToggleDisplayMode},
                    {"Memory Display", MemoryDisplay},{"Save Into File",SaveIntoFile},
                    { "Memory Modify", MemoryModify},{"Quit",Quit},{ NULL,NULL}};
    int menu_len = 9;
    state* s = calloc (1 , sizeof(state));
    s->unit_size = 1;
    while (1)
    {
        print_menu(menu, menu_len, s);
        // get user intput
        int option;
        scanf("%d", &option);
        if(option >= 0 && option < menu_len){
           printf("Within bounds \n"); 
           menu[option].fun(s);
        }else
            printf("Not within bounds \n");

    }
}