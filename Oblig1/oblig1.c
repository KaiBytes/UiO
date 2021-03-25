#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//gautesl@uio.no
//ojekblom@uio.no


//argc counts number of characters passed to the program. iow we have an array of chars, not of strings
int main(int argc, char *argv[]){ 
    


    //all arguments are strings, but variable will only contain memory address of first char in the string
    //why is it not char *str = &argv[1] --> you cannot get the address of an address! argv is an array of pointers

    char *str = argv[1]; //contains address of first element of the *argv[] char pointer array
    printf("printing out char: %s\n", str);
    printf("printing out char: %c\n", str[3]); //there is a value 3 places from address of str, fetch that // follow pointer and access values stored at 3 index location from address
    printf("printing out char: %c\n", *str);
    printf("printing out char: %c\n", *(str + 3)); //Dette her det samme som paa linje 19. Use address three locations from address stored in str and dereference
    //follow piointer to address + 3 and get valuje

    //printf("printing out char: %c\n", str[0]);   vs printf("printing out char: %c\n", *str);   
    char *toBeChanged = argv[2];
    printf("printing out toBeChanged: %c\n", *toBeChanged);
    char *changeTo = argv[3];
    // for (int i = 1; i < argc; ++i) {
    //     printf("printing out char: %c\n", *str);
    //     if (*argv[i] + i != 0){
    //         printf("printing out char: %c\n", *(argv[i] + 1));
    //     }
    //     printf("printing out pointer: %p\n", argv[i]);
    // }

    int indexCounter = 0;
    while (*(str + indexCounter) != 0){
        printf("printing out char: %c\n", *(str+indexCounter));
        printf("printing out char: %d\n", *(str+indexCounter));
        indexCounter++;
        if (*(str + indexCounter) == *toBeChanged){
            printf("is true\n");    
            *(str + indexCounter) = *changeTo;
        }
    }

    printf("printing out toBeChanged: %s\n", str);

    return 0;



}