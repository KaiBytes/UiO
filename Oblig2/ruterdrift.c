//nmemb = This is the number of elements, each oneMask with a size of size bytes.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define ENDRINGSNUMMER_MASK 0b11110000
#define AKTIVBIT_MASK 0b00000001
#define TRAADLOES_MASK 0b00000010
#define FEMGHZ_MASK 0b00000100
#define UBRUKT_MASK 0b00001000

struct ruter {
    //numbers based on the assumption that most systems are 64 bits

    int ruterId; //4bytes
    unsigned char flagg; // 1byte
    char *modellNavn; //8 bytes
    struct ruter **adjacencyArray; //8 bytes
    int visited; //4bytes
};

struct ruter **ruterArray;
int N, removed = 0, numberOfRouters;

struct ruter *getRuterById(int id){
    //Accounting for null pointers in ruerarray
    for(int i = 0; i < N; i++){
        if(ruterArray[i] != NULL && ruterArray[i]->ruterId == id) return ruterArray[i];
    }
    return NULL;
}

//add all adjacent routers to a router
//1. while there is a parent id
//2. read adjacent id
//3. if no adjacent id found == invalid list
//4. else add pointer to router to adjacency pointer array of parent router
void addAdjacentRuter(FILE *file){
    
    printf("***FILLING ADJACENCY LISTS***\n");

    int tmpParentId, tmpAdjacentId, rc;
    struct ruter *tmpParentRuter;

    while(fread(&tmpParentId, sizeof(int), 1, file) != 0){    
        rc = fread(&tmpAdjacentId, sizeof(int), 1, file);
        if(rc == 0) {
            fprintf(stderr, "No adjacent ruter found!\nTERMINATING WHILE LOOP");
            break;
        }

        tmpParentRuter = getRuterById(tmpParentId);
        for(int i = 0; i < 10; i++){
            if(tmpParentRuter->adjacencyArray[i] == NULL) {

                tmpParentRuter->adjacencyArray[i] = getRuterById(tmpAdjacentId);
                break;

            }
        }
        
    }

}


struct ruter* lag_ruter(char *name, unsigned char flags, int id){
    
    struct ruter *tmp = malloc(sizeof(struct ruter)); //25 bytes of memory allocated
    tmp->ruterId = id;
    tmp->flagg = flags;
    tmp->modellNavn = strdup(name);
    if (tmp->modellNavn == NULL) {
        perror("strdup");
        free(tmp);
        exit(EXIT_FAILURE);
    }

    if ((tmp->adjacencyArray = calloc(10, sizeof(void *))) == NULL) { //allocate 10 * 25 bytes of memory to adjacency array == 250 bytes
        fprintf(stderr, "Malloc failed\n");
        exit(EXIT_FAILURE);
    } 

    tmp->visited = 0;

    return tmp;
}

// reads in file passed to function as argoneMask
// 1. attempt to open file
// 2. when succesfull , read first integr byte which represents ruter array size
// 3. allocate memory to ruter array
// 4. read information about routers, create router structs and add pointer to struct in router array
// 5. add adjacent routers to each router
// 6. close file
void les_fil(char *argoneMask){
    
    FILE *f;
    int i, tmpId;
    unsigned char tmpFlags, modellLengde;
    char tempName[249];
    
    f = fopen(argoneMask, "rb");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fread(&N, sizeof(int), 1, f);
    if ((ruterArray = malloc(((sizeof(struct ruter*) + (sizeof(struct ruter) * 10)) * N))) == NULL) { //allocate 258 * N bytes to ruter array
        fprintf(stderr, "Malloc failed\n");
        exit(EXIT_FAILURE);
    } else fprintf(stdout, "malloc succeeded\n");

    for(i = 0; i < N; i++){
  
        //fetch information
        fread(&tmpId, sizeof(int), 1, f);
        fread(&tmpFlags, sizeof(char), 1, f);
        fread(&modellLengde, sizeof(char), 1, f);
        fread(tempName, sizeof(char), modellLengde, f);
        tempName[modellLengde] = 0;
        fgetc(f); //checks off final nullbyte

        ruterArray[i] = lag_ruter(tempName, tmpFlags, tmpId);
    }

    addAdjacentRuter(f);

    if (ferror(f)) {
        fprintf(stderr, "fread failed\n");
        exit(EXIT_FAILURE);
    }

    fclose(f);
}

//call free() on entire array
void free_array(){

    printf("***FREEING MEMORY***\n");
    int arrayLength =N;
    for(int i = 0; i < arrayLength; i++){
        if(ruterArray[i] == NULL) continue; //taking account for gaps caused by free_ruter
        free(ruterArray[i]->modellNavn); //frees 8 bytes allocated on line 75 for every i
        free(ruterArray[i]->adjacencyArray); //frees 250 bytes allocated on line 82 for every i
        free(ruterArray[i]); //frees 25 bytes allocated on line 72 for every i
    }
    free(ruterArray); //frees 258 * n bytes allocated on line 113
}

// write to new_topology.dat
// 1. try to open file
// 2. write numberOfRouters to new file
// 3. iterate over ruterArray and write information of every router to the file. I followed the formating of the oblig.
// 4. iterate over ruterarray again. This time I also iterate over the adjacency array. I add the ID's of every couple of parent and child routers to the file.
// 5. close files
void skriv_til_fil(){
    
    printf("***WRITING TO FILE***\n\n");
    
    int i, j, rc, nameLength, parentRouterId, childRouterId;
    char nullByte = '\0';
    numberOfRouters = N - removed;
    

    FILE *newF;
    if(!(newF = fopen("new-topology.dat", "wb"))){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if((rc = fwrite(&numberOfRouters,sizeof(int),1,newF)) == 0){
        perror("fwrite");
        fprintf(stderr, "Array length was not appended to the binary file");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < N; i++){

        if(ruterArray[i] == NULL) continue;

        if((rc = fwrite(&ruterArray[i]->ruterId,sizeof(int),1,newF)) == 0){
            perror("fwrite");
            fprintf(stderr, "ruterId #%d was not appended to the binary file", i);
            exit(EXIT_FAILURE);
        }

        if((rc = fwrite(&ruterArray[i]->flagg,sizeof(char),1,newF)) == 0){
            perror("fwrite");
            fprintf(stderr, "flagg #%d was not appended to the binary file", i);
            exit(EXIT_FAILURE);
        }

        nameLength = strlen(ruterArray[i]->modellNavn);
        if((rc = fwrite(&nameLength,sizeof(char),1,newF)) == 0){
            perror("fwrite");
            fprintf(stderr, "nameLength #%d was not appended to the binary file", i);
            exit(EXIT_FAILURE);
        }

        if((rc = fwrite(ruterArray[i]->modellNavn,sizeof(char), nameLength,newF)) < nameLength){
            perror("fwrite");
            fprintf(stderr, "name  #%d was not appended to the binary file", i);
            exit(EXIT_FAILURE);
        }

        if((rc = fwrite(&nullByte,sizeof(char), 1 ,newF)) == 0){
            perror("fwrite");
            fprintf(stderr, "flagg #%d was not appended to the binary file", i);
            exit(EXIT_FAILURE);
        }
    }

    for(i = 0; i < N; i++){
        
        if(ruterArray[i] == NULL) continue;
        
        parentRouterId = ruterArray[i]->ruterId;
        for (j = 0; j<10; j++){
            
            if(ruterArray[i]->adjacencyArray[j] == NULL) continue;

            childRouterId = ruterArray[i]->adjacencyArray[j]->ruterId;

            if((rc = fwrite(&parentRouterId,sizeof(int),1,newF)) == 0){
                perror("fwrite");
                fprintf(stderr, "parentID #%d was not appended to the binary file", parentRouterId);
                exit(EXIT_FAILURE);
            }

            if((rc = fwrite(&childRouterId,sizeof(int),1,newF)) == 0){
                perror("fwrite");
                fprintf(stderr, "childId #%d was not appended to the binary file", childRouterId);
                exit(EXIT_FAILURE);
            }

        }
    }

    fclose(newF);
}

//free oneMask specific router. Had to create this file as a helper function to slett ruter.
//it takes a struct pointer as an argument and free memory of modellnavn and the adjacency array before it frees the struct itself.
//frees up in total 283 bytes of memory
void free_ruter(struct ruter *rmRuter){
    printf("***FREEING ROUTER %d***\n\n", rmRuter->ruterId);
    free(rmRuter->modellNavn); //frees up 8 bytes allocated on line 75
    free(rmRuter->adjacencyArray); //frees up 250 bytes allocated on line 82
    free(rmRuter); //frees up 25 bytes allocated on line 72
}

//helper method to manipulate the endringsnummer part of the flag.
//the methos takes a pointer to the flag as an argument
//1. defined and intialized a mask which represent the number 1 in the endringsnummer part of the flag
//2. while the bitwise and operation of the flag and oneMask is bigger than 0, perform an XOR operation on flags with oneMask.
//3. the point is to find the right most 0, flip it to 1, and switch everything else to the left to 0.
//4. if the bitwise and operation of the flag and oneMask == 0, than just change the 4th bit to 1 using a XOR operation.
void bitwise_increment_by_one(unsigned char *flags){
    unsigned char oneMask = 0b00010000;
    while(*flags & oneMask){
        *flags ^= oneMask;
        oneMask <<= 1;
    }
    *flags ^= oneMask;
}

//removes a single router based on ID number
//1. check if id exists in ruterarray
//2. if yes, remove pointer refrence in adjacency list of every other router and increment endringsnumbr of these routers
//3. free the referenced router itself.
void slett_ruter(int Id){

    printf("***SLETT RUTER***\n\n");

    int i, j;
    struct ruter *tmp = getRuterById(Id), *finalAdjacencyElement = NULL;

    if (tmp == NULL){
        perror("getRuterById");
        printf("Ruteren finnes ikke");
    } else {
        for(i = 0; i < N; i++){
            //removing from adjacency list
            for(j = 9; j >= 0; j--){
                if(ruterArray[i]->adjacencyArray[j] != NULL && finalAdjacencyElement == NULL){
                    finalAdjacencyElement = ruterArray[i]->adjacencyArray[j];
                    // printf("%d\n\n", finalAdjacencyElement->ruterId);
                    ruterArray[i]->adjacencyArray[j] = NULL;
                }
                if(ruterArray[i]->adjacencyArray[j] != NULL && ruterArray[i]->adjacencyArray[j]->ruterId == Id){
                    ruterArray[i]->adjacencyArray[j] = finalAdjacencyElement;
                }
            }

            //removing from ruterarray or increment endringsnummer
            if (ruterArray[i]->ruterId == Id){
                ruterArray[i] = NULL;
            } else bitwise_increment_by_one(&ruterArray[i]->flagg);

            //resetting for next run
            finalAdjacencyElement = NULL;
        }

        free_ruter(tmp); //frees 283 bytes of memory with gunction defined on line 250
        removed += 1;
    }

}


//add connection based to router based on id arguments passed
//1. check if routers with those id exist
//2. if yes, increment endringsnumbero of parent by one and add child router on empty space in adjacency list
void legg_til_kobling(int Id1, int Id2){
    
    printf("***LEGG TIL KOBLING***\n\n");
    struct ruter *tmpOne = getRuterById(Id1);
    struct ruter *tmpTwo = getRuterById(Id2);
    if (tmpOne == NULL || tmpTwo == NULL){
        perror("getRuterById");
        printf("Ruteren finnes ikke");
    } else {
        // printf("%d\n", tmpOne->ruterId);
        // unsigned char tmpFlagg = tmpOne->flagg >> 4;
        // printf("%d\n", tmpFlagg);
        bitwise_increment_by_one(&tmpOne->flagg);
        // printf("%d\n\n", tmpOne->flagg >> 4);
        for(int i = 0; i < 10; i++){
            if (tmpOne->adjacencyArray[i] == NULL){
                tmpOne->adjacencyArray[i] = tmpTwo;
                break;
            }
        }
    }

}

//if tmp exists print, else get lost.
void print(int Id){
    
    struct ruter *tmp = getRuterById(Id);
    
    //Accounting for null pointers in ruerarray
    if (tmp != NULL){
        printf("name : %s\nid : %12d\nflag : %10d\n", tmp->modellNavn, tmp->ruterId, tmp->flagg);
    
        printf("Neighbours : ");
        
        for (int i = 0; tmp->adjacencyArray[i] != NULL; i++){
            printf("%4d ", tmp->adjacencyArray[i]->ruterId);
        }

        printf("\n\n");
    } else {
        perror("getRuterById");
        printf("Ruteren finnes ikke\n");
    }
}

//gives router new name. arguments are an id and new name
//1. check if routr with that id exists
//2.if yes , first free original modellnavn and add new modellNavn using strdup.
void sett_modell(int Id, char *newName){
    
    printf("***NEW NAME***\n\n");
    
    struct ruter *tmp = getRuterById(Id);
    if (tmp == NULL){
        perror("getRuterById");
        printf("Ruteren finnes ikke");
    } else {
        free(tmp->modellNavn); //frees 8 bytes of memory allocated in lag ruter for this variable.
        tmp->modellNavn = strdup(newName);
        if (tmp->modellNavn == NULL) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
    }

}

enum type {
    AKTIV = 0,
    TRAADLOES = 1,
    FEMGHZ = 2,
    UBRUKT = 3,
    ENDRINGSNUMMER = 4,
};

//flips bits of the flag
//id refers to router id, flag refers to which flag we want to chang, value is the value we want to use.
//1. check if router with id exists.
//2. if yes, do some checks whether flags and values are valid.
//3. if yes, flip the bits of the flags to the desired values, else display error message
//OBS! I used the enum defined above to do the check. Just wanted to test
void sett_flagg(int Id, int flag, int verdi){
    struct ruter *tmp = getRuterById(Id);
   
    
    if(tmp == NULL) printf("!!! THERE IS NO ROUTER WITH THIS ID !!!");
    
    switch(flag) {
        case AKTIV:

            if (verdi > 1) {
                printf("CANNOT USE THIS VALUE FOR AKTIVBIT\n");
            } else {
                printf("ACTIVITY BIT WAS %d\n", (tmp->flagg & AKTIVBIT_MASK));
                if (verdi != (tmp->flagg & AKTIVBIT_MASK)) tmp->flagg ^= AKTIVBIT_MASK;
                printf("ACTIVITY BIT IS NOW %d\n", (tmp->flagg & AKTIVBIT_MASK));
            }
            
            break;

        case TRAADLOES:

            if (verdi > 1) {
                printf("CANNOT USE THIS VALUE FOR TRAADLOESBIT\n");
            } else {
                printf("TRAADLOES BIT WAS %d\n", ((tmp->flagg & TRAADLOES_MASK)>>1));
                if (verdi != (tmp->flagg & TRAADLOES_MASK)>>1) tmp->flagg ^= TRAADLOES_MASK;
                printf("TRAADLOES BIT IS NOW %d\n", ((tmp->flagg & TRAADLOES_MASK)>>1));
            }

            break;

        case FEMGHZ:

            if (verdi > 1) {
                printf("CANNOT USE THIS VALUE FOR FEMGHZBIT\n");
            } else { 
                printf("FEMGHZ BIT WAS %d\n", ((tmp->flagg & FEMGHZ_MASK)>>2));
                if (verdi != (tmp->flagg & FEMGHZ_MASK)>>2) tmp->flagg ^= FEMGHZ_MASK;
                printf("FEMGHZ BIT IS NOW %d\n", ((tmp->flagg & FEMGHZ_MASK)>>2));
            }
        
            break;

        case UBRUKT:

            printf("!!! Position 3 is unused. No changes have been made !!!\n");
            break;

        case ENDRINGSNUMMER:
            
            if(verdi <= 15) {
                
                printf("ENDRINGSNUMMER WAS %d\n", (tmp->flagg & ENDRINGSNUMMER_MASK)>>4);

                while((tmp->flagg & ENDRINGSNUMMER_MASK)>>4 != verdi){
                    // if ((tmp->flagg & ENDRINGSNUMMER_MASK)>>4 == 0 ) (tmp->flagg & ENDRINGSNUMMER_MASK) >>4 = 0b00000000;
                    bitwise_increment_by_one(&tmp->flagg);
                }

                printf("ENDRINGSNUMMBER is %d\n", ((tmp->flagg & ENDRINGSNUMMER_MASK)>>4));
            } else printf("!!! CANNOT SET %d AS A VALUE FOR ENDRINGSNUMMER!!!\n", verdi);
            
            break;
        
        default:
            fprintf(stderr, "!!! THIS IS NOT A VALID BIT POSITION !!!\n");
    }
}

//self explanatory.
void reset_visited(){
    for (int i =0; i<N;i++){
        if(ruterArray[i]==NULL) continue;
        else ruterArray[i]->visited = 0;
    }
}

//find connection between two routers with different id's
//1. check if router with parentId exists
//2. if yes, set router variable visisted to 1. this is to avoid it being visisted twice on future recursive calls.
//3. continue to iterate over adjacency list. If we find a non NULL value in the list, we have found a router. Check if it has been visited
//4. if yes, check if id2 matches with router id -> y? match n? recursive call
void finnes_rute(int Id1, int Id2){
    int i;
    struct ruter *tmpParent = getRuterById(Id1);
    struct ruter *tmpChild;
    if (tmpParent == NULL) {
        perror("getRuterById");
        printf("Ruteren finnes ikke");
    } else{
        tmpParent->visited = 1;
        for(i = 0; i < 10; i++){
            if((tmpChild = tmpParent->adjacencyArray[i]) == NULL){
                printf("Ruteren #%d har ikke noe rutere i adjacencylist lenger\n\n", tmpParent->ruterId);
                break;
            } else if ((tmpChild = tmpParent->adjacencyArray[i]) != NULL && tmpChild->visited == 0){
                if(tmpChild->ruterId == Id2) {
                    printf("CONNECTION FOUND!\n");
                    printf("%d -> %d\n\n", tmpParent->ruterId, tmpChild->ruterId);
                    break;
                } else {
                    finnes_rute(tmpChild->ruterId, Id2);
                }
            }
        }
        
    }
}

//read command list
//1. check if we can ope nfile
//2. define buffer for each line going to be read from the file
//3. call fgets and copy line to buffer, while we can read a line from the file (!EOF) this loop wont stop
//4. define char array which will store values from sscanf
//5. sscanf will look for pattern. it returns the amount of elements it was able to match. this value is stored in rc.
//5. based on rc and the command string i am able to distinguish between commands. I call the relevant function with the arguments I was able to store in each sscanf round.
//6. close file
void les_kommandoer(char *file){
    
    printf("\n***READING COMMANDS***\n\n");


    FILE *f = fopen(file, "r");
    if (!f){
        perror("fopen");
        fprintf(stderr, "could not open commands file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    

    while (fgets(line, sizeof(line), f) != NULL){
        
        char command[128], argoneMask[128], argTwo[128], argThree[128], newName[128], rc;

        rc = sscanf(line, "%s %s %s %s", command, argoneMask, argTwo, argThree);        

        if (rc == 2){
            if (strcmp(command, "slett_ruter") == 0){
                printf("*** SLETT COMMAND FOUND! DELETING ROUTER WITH ID #%d ***\n\n", atoi(argoneMask));
                slett_ruter(atoi(argoneMask));

            } else {
                printf("*** PRINTING COMMAND FOUND! PRINTING OUT ROUTER WITH ID #%d ***\n\n", atoi(argoneMask));
                print(atoi(argoneMask));

            }
            

        } else if (rc == 3){
            if (strcmp(command, "sett_modell") == 0){
                printf("*** SETT_MODELL COMMAND FOUND! SETTING NAME %s FOR RUTER WITH ID #%d ***\n\n", argTwo, atoi(argoneMask));
                sett_modell(atoi(argoneMask), argTwo);

            } else if (strcmp(command, "finnes_rute") == 0){
                printf("*** FINNES_RUTE COMMAND FOUND! FINDING CONNECTION BETWEEN %d AND %d ***\n\n", atoi(argoneMask), atoi(argTwo));
                finnes_rute(atoi(argoneMask), atoi(argTwo));
                reset_visited();

            } else if (strcmp(command, "legg_til_kobling") == 0){
                printf("*** LEGG_TIL_KOBLING COMMAND FOUND! ADDING CONNECTION WITH ID #%d TO ROUTER WITH ID #%d ***\n\n", atoi(argoneMask), atoi(argTwo));
                legg_til_kobling(atoi(argoneMask), atoi(argTwo));
            } 

        } else if (rc == 4){
            if (strcmp(command, "sett_modell") == 0){
                strcpy(newName, argTwo);
                strcat(newName, " ");
                strcat(newName, argThree);
                printf("*** SETT_MODELL COMMAND FOUND! SETTING NAME %s FOR RUTER WITH ID #%d ***\n\n", newName, atoi(argoneMask));
                sett_modell(atoi(argoneMask), newName);

            } else if (strcmp(command, "sett_flagg") == 0){
                printf("*** SETT_FLAGG COMMAND FOUND! SETTING FLAG %d FOR RUTER WITH ID #%d TO VALUE %d ***\n\n", atoi(argTwo), atoi(argoneMask), atoi(argThree));
                sett_flagg(atoi(argoneMask), atoi(argTwo), atoi(argThree));

            }
        }
    }

    fclose(f);

}

//no explanation necessary
int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Error: Trenger two argumenter.\n");
    } else if (argc > 3){
        printf("Error: for mange argumenter.\n");
    } else {

        les_fil(argv[1]);
        
        printf("***\nBEFORE COMMANDS***\n");
        for (int i = 0; i < N; i++){
            if(ruterArray[i] == NULL) continue;
            print(ruterArray[i]->ruterId);
        }

        les_kommandoer(argv[2]);

        printf("***AFTER COMMANDS***\n");
        for (int i = 0; i < N; i++){
            if(ruterArray[i] == NULL) continue;
            print(ruterArray[i]->ruterId);
        }

        skriv_til_fil();

        free_array(); //frees all memory allocated during the program. method is defined on line 142. I made calculkatiosn there about how many bytes are going to be freed.
    }

    return EXIT_SUCCESS;
}