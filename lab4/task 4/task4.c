#include <stdio.h>
int count_digits(char *argv)
{
    int num = 0;
    unsigned int i = 0;
    while(argv[i] != '\0' )
    {
        if (argv[i] >= '0' && argv[i] <= '9')
            num++;
        i++;
    }
    return num;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Less arguements than needed.\n");
        return 1;
    }
    printf("the number is %d\n", count_digits(argv[1]));
    return 0;
}