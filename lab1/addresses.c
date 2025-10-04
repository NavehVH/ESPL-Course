#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Global variables: Their location is in the Data Segment,
int addr5;
int addr6;

//functions are in the code segment
int foo()
{
    return -1;
}
void point_at(void *p);
void foo1(); //function addresses are in the code segment. they are usually the lowest.
char g = 'g';
void foo2();

int secondary(int x)
{
    //Local variables: Their location is in the Stack, generally in high memory addresses
    int addr2;
    int addr3;
    char *yos = "ree"; //String Literals are read-only data segment
    int *addr4 = (int *)(malloc(50)); //Dynamically Allocated memory in the Heap. betweeb tge stack and data segment addresses
	int iarray[3];
    float farray[3];
    double darray[3];
    char carray[3]; 
	int iarray2[] = {1,2,3};
    char carray2[] = {'a','b','c'};
    int* iarray2Ptr;
    char* carray2Ptr; 
    
	printf("- &addr2: %p\n", &addr2);
    printf("- &addr3: %p\n", &addr3);
    printf("- foo: %p\n", &foo);
    printf("- &addr5: %p\n", &addr5);
	printf("Print distances:\n");
    point_at(&addr5);

    printf("Print more addresses:\n");
    printf("- &addr6: %p\n", &addr6);
    printf("- yos: %p\n", yos);
    printf("- gg: %p\n", &g);
    printf("- addr4: %p\n", addr4);
    printf("- &addr4: %p\n", &addr4);

    printf("- &foo1: %p\n", &foo1);
    printf("- &foo1: %p\n", &foo2);
    
    printf("Print another distance:\n");
    printf("- &foo2 - &foo1: %ld\n", (long) (&foo2 - &foo1));


    /* task 1 b here */
    printf("Pointers and arrays (T1d): "); 
    //the + operator increments the pointer by the app' number of bytes required for the specific data type.
    //for exmaple iarray + 1 will point to the memory location immediately after the last element of iarray, and increase by 4 bytes 
    //char for example (carray + 1), will jump only by 1 byte

    printf("Pointer to iarray: %p\n", (void *)iarray); //int 4 bytes
    printf("Pointer to iarray+1: %p\n", (void *)(iarray + 1));

    printf("Pointer to farray: %p\n", (void *)farray); //float 4 bytes
    printf("Pointer to farray+1: %p\n", (void *)(farray + 1));

    printf("Pointer to darray: %p\n", (void *)darray); //double 8 bytes
    printf("Pointer to darray+1: %p\n", (void *)(darray + 1));

    printf("Pointer to carray: %p\n", (void *)carray); //char 1 byte
    printf("Pointer to carray+1: %p\n", (void *)(carray + 1));
    
    printf("Pointers and arrays (T1d): ");

    /* task 1 d here */
    

}

//command-line arguments like argc, argv are in the Stack
int main(int argc, char **argv)
{ 

    
    printf("Command line arg addresses (T1e):\n");
    /* task 1 e here */

    printf("Print function argument addresses:\n");
    printf("- &argc %p\n", &argc); //local var, so in stack
    printf("- &argv %p\n", &argv); //local var, so in stack
	
	secondary(0);
    for (int i = 0; i < argc; i++) {
        printf("- argv[%d] Address: %p, Content: %s\n", i, &argv[i], argv[i]); //in the heap
    }
    return 0;
}

void point_at(void *p) //he used a global var as parameter addr5 (location at the data segment)
{
    //local variables are on the stack, they change in each function call, generally in high memory addresses
    int local;

    //static var are in the data segment. This addresses lower than heap and stack add' but higher than the code segment
    static int addr0 = 2;
    static int addr1;

    long dist1 = (size_t)&addr6 - (size_t)p; //their distance is 4, they are both global and declared one after another. Its an int so their distance is 4
    long dist2 = (size_t)&local - (size_t)p; //local vars are higher from global ones. local are in the stack, global in the data segment
    long dist3 = (size_t)&foo - (size_t)p;

    printf("- dist1: (size_t)&addr6 - (size_t)p: %ld\n", dist1); //4
    printf("- dist2: (size_t)&local - (size_t)p: %ld\n", dist2); //minus 1m
    printf("- dist3: (size_t)&foo - (size_t)p:  %ld\n", dist3); //minus 10k
    
    printf("Check long type mem size (T1a):\n");
    /* part of task 1 a here */

    //Numerical Values and Order
    
    //HIGHEST
    //stack: local vars within func. like local, addr3. USUALLY grows downwards in most systems.
    //heap: dynamic memory like addr4, 
    //data segment: contains global and static vars. like addr5, addr6
    //code segment (TEXT SEGMENT, usually read only): program compiled code. Like foo, foo1, foo2 addresses. if we have fun(a, b, c) will enter c after that b after that a
    //LOWEST 


   printf("Size of long: %lu\n", sizeof(long));
   //long is 4 byes, so can go to values of −2^31 to 2^31−1. long type is not sufficient to represent all possible address differences on a 32-bit system.
   //can use unsigned long, which sufficient to cover all address range, but cant represent negative differences
   


    printf("- addr0: %p\n", &addr0);
    printf("- addr1: %p\n", &addr1);
}

void foo1()
{
    printf("foo1\n");
}

void foo2()
{
    printf("foo2\n");
}