#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//argc counts number of characters passed to the program. iow we have an array of chars, not of strings
int main(int argc, char *argv[]){ 
    
    //all arguments are strings, but variable will only contain memory address of first char in the string
    //why is it not char *str = &argv[1] --> you cannot get the address of an address! argv is an array of pointers


    //fetching arguments
    char *str = argv[1]; //contains address of first element of the *argv[] char pointer array
    char *toBeChanged = argv[2];
    char *changeTo = argv[3];

    //Switch dem CHARSSSSSSSSSSSSS
    int indexCounter = 0;
    while (*(str + indexCounter) != 0){
        indexCounter++;
        if (*(str + indexCounter) == *toBeChanged){
            *(str + indexCounter) = *changeTo;
        }
    }

    printf("printing out toBeChanged: %s\n", str);

    return 0;



}