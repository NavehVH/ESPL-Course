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


void print_menu(fun_desc menu[], int menu_length, state* s){
    if(debug == 1){
        fprintf(stderr,"unit_size: %d\n", s->unit_size);
        fprintf(stderr,"file_name: %s\n", s->file_name);
        fprintf(stderr,"mem_count: %d\n", s->mem_count);
    }

    printf("Choose action\n");
    for (int i = 0; i < menu_length; i++)
        printf("%d- %s\n", i, menu[i].name);
    printf("Option :\n");
}

void ToggleDebugMode(state* s){
    if(debug == 0){
        debug = 1;
        fprintf(stderr,"Debug flag now on\n");
    }
    else{
        debug = 0;
        fprintf(stderr,"Debug flag now off\n");
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

void LoadIntoMemory() {
    printf("Load Into Memory: Not implemented yet\n");
}

void ToggleDisplayMode() {
    printf("Toggle Display Mode: Not implemented yet\n");
}

void MemoryDisplay() {
    printf("Memory Display: Not implemented yet\n");
}

void SaveIntoFile() {
    printf("Save Into File: Not implemented yet\n");
}

void MemoryModify() {
    printf("Memory Modify: Not implemented yet\n");
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