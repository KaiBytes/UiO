#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#include "rdp.h"
#include "send_packet.h"

//port and ip address to send to
// #define IP "127.0.0.1"
// #define port "2022"

void fsp_connectPacket(struct rdp_packet *packet, int id){
    packet->flags = 0x01; //connect request flag
    packet->pktseq = '\0'; //null char has all bits set to zero, will need to do bitwise operations
    packet->ackseq = '\0';
    packet->unassigned = '\0';
    packet->senderid = (int) htons(id); //network byte order
    packet->recvid = (int) htons(0); //network byte order
    packet->metadata = htons(0);
    // rdp_read(NULL, packet);
}

int main(int argc, char *argv[]){

    char *ip, *port, *probability;
    if (argc < 4) {
        printf("Error: Trenger tre argumenter.\nIP, port, probability\n");
        return EXIT_FAILURE;
    } else if (argc > 4){
        printf("Error: for mange argumenter.\nIP, port, probability\n");
        return EXIT_FAILURE;
    }

    // setting up all base variables to be used in this main method

    ip = argv[1];
    port = argv[2];
    probability = argv[3];
    srand(time(0));                                 //used to get random integer for id assignment
    int clientFd, rc, randId = rand() % 100;
    set_loss_probability(atof(probability));        
    
    // BUILDING FILENAME
    char connectionId[3];
    sprintf(connectionId, "%d", randId);
    char fileName[128] = "kernel-file-";
    strcat(fileName, connectionId);
    strcat(fileName, ".pdf");
    printf("%s\n", fileName);

    //FILE TRANSFER VARIABLES
    fd_set fds;
    FILE *trgtFile = fopen(fileName, "ab+");
    if (!trgtFile) {
        perror("fopen");
        fprintf(stderr, "[TRGTFILE] UNABLE TO OPEN FILE!\n");
        exit(EXIT_FAILURE);
    }

    //SERVER ADDRESS REFERENCE
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    rc = getaddrinfo( ip, port, &hints, &res);
    if(rc != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

// SENDING CONREQUEST ------------------------------------------------------------------------------------------------------

    //setting up connect request packet
    struct rdp_packet *currentPacket = calloc(1, sizeof(struct rdp_packet) + 1000);
    fsp_connectPacket(currentPacket, randId);
    printf("print char pointer : %s\n", (char *)currentPacket);
    
    //sending connect request to server
    clientFd = rdp_connect( res->ai_addr, res->ai_addrlen, currentPacket); //works
    check_error(clientFd, "clientFd");

    rdp_printPacket(currentPacket);

// MAIN EVENT LOOP CONREQUEST ----------------------------------------------------------------------------------------------

    //used in main eventloop
    struct timeval timeout;
    memset(&timeout, 0, sizeof(struct timeval));

    int isRunning = 1;
    //runs if connect request succeeded
    if(clientFd){
        while(isRunning){
            FD_ZERO(&fds);

            timeout.tv_sec = 1;

            FD_SET(clientFd, &fds);

            rc = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
            
            //only deal with package when  (packet sequence > acked sequence)
            if(FD_ISSET(clientFd, &fds)){

                struct rdp_packet *tmpPacket = calloc(1, sizeof(struct rdp_packet) + 1000);
                
                rc = recv(clientFd, tmpPacket, (sizeof(struct rdp_packet)+1000) ,0);
                check_error(rc, "recv");


                switch(tmpPacket->flags){
                    case DATA:            
                        
                        //check for resend -> currentPackage ackseq < tmppakct pktseq? = new packet
                        //
                        if(tmpPacket->pktseq > currentPacket->ackseq && ntohs(tmpPacket->metadata) != 0){
                            printf("TRUE\n");
                            free(currentPacket);
                            currentPacket = tmpPacket;

                            rdp_printPacket(currentPacket);

                            //write to file
                            rdp_write(currentPacket, trgtFile);
                            
                            //set packet to 0x80 and ackseq increment by one
                            currentPacket->flags = 0x08;
                            rdp_bitwiseIncrement(&currentPacket->ackseq);
                            rdp_idSwitch(currentPacket);
                            currentPacket->metadata = htons(0);
                            
                            //send to server
                            rc = send_packet(clientFd, (char *)currentPacket, (sizeof(struct rdp_packet) + 1000),0, res->ai_addr, res->ai_addrlen);
                            check_error(rc, "send_packet");

                            rdp_printPacket(currentPacket);
                        } else if (tmpPacket->pktseq == currentPacket->ackseq) { //SOURCE OF BUG! SENDS ACK TWICE -> ACKSEQ IS INCREMENTED ABOVE

                            // currentPackage ackseq == tmppakct pktse
                            //packet has been resend, just sent previous confirmation.
                            //currentPacket remains unchanged
                            rc = send_packet(clientFd, (char *)currentPacket, (sizeof(struct rdp_packet) + 1000),0, res->ai_addr, res->ai_addrlen);
                            free(tmpPacket);

                        } else if (ntohs(tmpPacket->metadata) == 0){
                            
                            printf("\n*** [EVENT LOOP] FINAL PACKET RECEIVED : SENDING TERMINATING PACKET***\n");
                            free(currentPacket);
                            currentPacket = tmpPacket;
                            currentPacket->flags = TERMINATE;
                            rdp_idSwitch(currentPacket);
                            currentPacket->metadata = htons(970);
                            rc = send_packet(clientFd, (char *)currentPacket, (sizeof(struct rdp_packet) + 1000),0, res->ai_addr, res->ai_addrlen);
                            free(tmpPacket);
                            isRunning = 0;
                            printf("TERMINATING CLIENT\n\n FILENAME : %s\n\n", fileName);
                        }
                        
                        break;
                    
                    default:
                        free(tmpPacket);
                    
                }

                
            }

            if(rc == 0){
                printf("waiting...\n");
            }
            
        }
        close(clientFd);
    } else {
        printf("*** [EVENT LOOP] CONNECT FAILED ***\n");
    }

    freeaddrinfo(res); //CLOSING SERVER ADDRESS REFERENCE STRUCT
    fclose(trgtFile); //CLOSING 

    return EXIT_SUCCESS;
}