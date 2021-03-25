#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int stringsum(char *s) { 
    int index = 0;
    int sum = 0;
    while(*(s + index) != 0){
        int tempInt = *(s+index);
        if(tempInt >= 65 && tempInt <= 90){
            sum += tempInt - 64;
        } else if (tempInt >= 97 && tempInt <= 122){
            sum += tempInt - 96;
        } else {
            return -1;
        }
        index ++;
    }
    return sum;
}

void stringsum2(char *s, int *res) { 
    
    int sum = stringsum(s);
    *res = sum;

}

int distance_between(char *s, char c) { 
    int indexFirstAppearance = -1;
    int indexSecondAppearance = -1;
    int indexCounter = 0;
    while(*(s + indexCounter) != 0){
        char tempchar = *(s+indexCounter);
        // printf("tempchar : %c", tempchar);
        if (tempchar == c && indexFirstAppearance < 0){
            indexFirstAppearance = indexCounter;
            // printf("firstAppearance : %d\n", indexFirstAppearance);
        } else if (tempchar == c && indexFirstAppearance >= 0){
            indexSecondAppearance = indexCounter;
            // printf("secondAppearance : %d\n", indexSecondAppearance);
            return (indexSecondAppearance - indexFirstAppearance);
        }
        indexCounter ++;
    }

    return -1;
}

char *string_between(char *s, char c) { 
    //determining size malloc needs to have using distance between function
    int distance = distance_between(s, c); 
    //a1234a distance = 5
    //we want malloc size of 4. malloc will allocate memory for an array and does this with help of indexlocations
    //in example above: we want to allocate memory for four chars + an end of string 0
    //we need an array array of 0 to 4

    //for loop was unable to find a distance. Variable c not found twice, returning NULL
    if(distance == -1){
        return NULL;
    }
    // if test passed, string between possible.
    // defining and initializing pointer to malloc with length defined in distance
    char *kopi; 
    kopi = malloc(sizeof(char*) * (distance-1)); //not to self, *kopi is freed in test file! :D
    for(int i = 0; i < distance; i++){
        kopi[i] = 65 + i;
    }
    //determining fist and second occurance of c
    // int i, lengde = strlen(s);
    int FirstAppearance = -1;
    int SecondAppearance = -1;
    int secondFound = -1;
    int tempIndex = 0;
    while(*(s+tempIndex) != 0 && secondFound != 0){
        if (*(s+tempIndex) == c && FirstAppearance < 0){
            FirstAppearance = tempIndex;
            // printf("firstAppearance : %d\n", indexFirstAppearance);
        } else if (*(s+tempIndex) == c && FirstAppearance >= 0 && secondFound == -1){
            SecondAppearance = tempIndex;
            // printf("secondAppearance : %d\n", indexSecondAppearance);
            secondFound = 0;
        }
        tempIndex++;
    }
    tempIndex = 0;
    for(int i = FirstAppearance + 1 ; i <= SecondAppearance - 1; i++){
        *(kopi + tempIndex) = *(s + i);
        // printf("currentChar: %c\n", *(s+i));
        tempIndex++;
    }

    // kopi[distance-1] = 0;
    *(kopi + (distance-1)) = 0;
    return kopi;
    

 }