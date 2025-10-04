/* $Id: count-words.c 858 2010-02-21 10:26:22Z tolpin $ */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

/*
how to use the gdb:
do a break point on main, 

*/

//Task 0:
//location of the segmentation fault is in the words function where you are attempting to modify a string literal, which is not allowed in C.
char *words(int count) {
    static char word[6]; 
    
    if (count == 1) {
        strcpy(word, "word");
    } else {
        strcpy(word, "words");
    }

    return word;
}

/* print a message reportint the number of words */
int print_word_count(char **argv)
{
  int count = 0;
  char **a = argv;
  while (*(a++))
    ++count;
  char *wordss = words(count);
  printf("The sentence contains %d %s.\n", count, wordss);
  
  return count;
}

/* print the number of words in the command line and return the number as the exit code */
int main(int argc, char **argv)
{
  print_word_count(argv + 1);
  return 0;
}