#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//T2a
//gets pointer to a char, integer and pointer to func, return new array allocated that each val f applied on it.
char* map(char *array, int arrayLength, char (*f) (char)){
  char* newArray = (char*)(malloc(arrayLength * sizeof(char)));//used chatgpt, allocate the memory
  for(int i = 0; i < arrayLength; i++){
    newArray[i] = f(array[i]); //do f on it
  }
  return newArray;
}


//T2b

//return char from stdin using fgetc
//stdin -  It allows C programs to read input from the user during runtime.
//fgetc - read a single character from a file stream
char my_get(char c){
  char input = fgetc(stdin);
  return input;
}


char cprt(char c){
  if(c > 0x20 && c < 0x7E){ //ASCII values
    printf("%c\n" , c); //print c
  }
  else{
    printf(".\n"); //print dot
  }
  return c;
}

char encrypt(char c){
  if(c > 0x20 && c < 0x4E){
    return c + 0x20; //add 0x20
  }
  else{
    return c; //else dont change
  }
}
 
char decrypt(char c){
  if(c > 0x40 && c < 0x7E){ 
    c = c - 0x20; //sub 0x20
  }
    return c; //else dont change
}

char xoprt(char c) {
    printf("%x  ", c); //hexadecimal
    
    printf("%o\n", c); //octal
    
    return c;
}

//T3b
struct fun_desc {
    char *name;
    char (*fun)(char);
};


//print menu (3)
void menuT(struct fun_desc menu[]) {
    printf("Select operation from the following menu:\n");
    for (int i = 0; menu[i].name != NULL; i++) {
        printf("%d) %s\n", i, menu[i].name);
    }
}


int main(int argc, char* argv[]) {
    char* carray = (char*)malloc(5 * sizeof(char));//used chatgpt (1), allocate a block of memory on the heap. So we are allocating 5*chars
    char *tempArray = NULL;
    carray[0] = '\0';//used chatgpt

    //the options, the functions (2)
    struct fun_desc menu[] = {
        {"Get string", my_get},
        {"Print string", cprt},
        {"Encrypt", encrypt},
        {"Decrypt", decrypt},
        {"Print hex and octal", xoprt},
        {NULL, NULL}
    };


    while (1) { //repeats forever until EOF called
        menuT(menu); //(3)
        char buffer[100]; //store input from the user
        int input;
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) { //reads a line from stdin and store into buffer, null means EOF
            free(carray); //prevent memor leak
            exit(0);
        }
        input = atoi(buffer);//used chatgpt, it converts a string into an integer

        if (input < 0 || input > 4) {
            printf("Not within bounds\n");
            free(carray);
            exit(1);
        }
        printf("Within bounds\n");
        tempArray = map(carray, 5, menu[input].fun);
        carray = tempArray; //(6)
        free(tempArray);

    }
    free(carray);

    return 0;
}

