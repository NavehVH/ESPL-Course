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
  

} state;

typedef struct fun_desc {
    char *name;
    void (*fun)(state*);
}fun_desc;

int debug = 0; 
int display = 0; 


void print_menu(fun_desc menu[], int menu_length, state* s){
    if(debug == 1){
        printf("unit_size: %d\n", s->unit_size);
        printf("file_name: %s\n", s->file_name);
        printf("mem_count: %d\n", s->mem_count);
    }

    printf("Choose action\n");
    for (int i = 0; i < menu_length; i++)
        printf("%d- %s\n", i, menu[i].name);
    printf("Option :\n");
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
        printf("Debug: file name set to %s\n", s->file_name);
}

void SetUnitSize(state* s){
    int new_unit;
    printf("insert new unit size: \n");
    scanf("%d", &new_unit);
    if(new_unit == 1 || new_unit == 2 || new_unit == 4){
        if(debug == 1)
            printf("Debug: set size to %d\n", new_unit);
        s->unit_size = new_unit;
    }
    else
        printf("invalid value\n");
}

void Quit(state* s){
    if(debug == 1)
        printf("quitting\n");
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
    printf("insert location and length (seperated by single space): \n");
    int length = 0;
    int location = 0;
    scanf("%x %d", &location, &length);
    if(debug == 1){
        printf("file name: %s\n",s-> file_name);
        printf("location: %d\n",location);
        printf("length: %d\n",length);
    }
    fseek(file, location, SEEK_SET);
    s->mem_count = s->unit_size * length;
    fread(s->mem_buf, s->unit_size, length, file);
    printf("Loaded %d uniuts into memory\n", length);
    fclose(file);

}
void printMem(void* buff, int count, int uSize){
    void* end = buff + uSize * count;
    char* dec_options[] = {"%hhd\n", "%hd\n", "ERROR", "%d\n"};
    char* hex_options[] = {"%hhx\n", "%hx\n", "ERROR", "%x\n"};
    while(buff < end){
        int curr = *((int*)(buff));
        if(display == 0){//dec
            printf(dec_options[uSize-1], curr);
        }else{
            printf(hex_options[uSize-1], curr);
        }
        buff += uSize;
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
    printf("insert src address, dest offset and length (seperated by single space): \n");
    int source_address = 0;
    int length = 0;
    int target_location = 0;
    scanf("%x %x %d", &source_address, &target_location, &length);
    fseek(file, 0L, SEEK_END);
    if (target_location > ftell(file)){
        printf("Error");
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
    printf("insert location and value (seperated by single space): \n");
    int location = 0;
    int val = 0;
    scanf("%x %x", &location, &val);
    if (debug == 1){
        printf("location: %x, val: %x\n",location, val);
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