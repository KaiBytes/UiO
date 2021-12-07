#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "rdp.h"
#include "send_packet.h"



int main(int argc, char *argv[]){

    //arguments
    char *port, *filename, *maxFiles, *probability;

    if (argc < 5) {
        printf("Error: Trenger fem argumenter.\nport, filename, maxFiles, probability\n");
        return EXIT_FAILURE;
    } else if (argc > 5){
        printf("Error: for mange argumenter.\nport, filename, maxFiles, probability\n");
        return EXIT_FAILURE;
    }

    port = argv[1];
    filename = argv[2];
    maxFiles = argv[3];
    probability = argv[4];
    
    //base variables
    int serverFd, rc, MAX = atoi(maxFiles), CURRENT = 0, FINISHED = 0; //current is icnremented in accept. FINISHED is incremented at line
    fd_set fds;
    set_loss_probability(atof(probability)); //atof converts string to float

    //listening socket
    serverFd = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(serverFd, "socket");

    //Construct local address structure
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(port));
    my_addr.sin_addr.s_addr = INADDR_ANY;

    //binds listening socket to a port on the server
    rc = bind(serverFd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    check_error(rc, "bind");

    //maintain list of clients connected to this server
    struct rdp_connection **clients;
    if ((clients = calloc(MAX, sizeof(void *))) == NULL) {
        fprintf(stderr, "Malloc failed\n");
        exit(EXIT_FAILURE);
    } 

    struct timeval eventTimeout;
    memset(&eventTimeout, 0, sizeof(struct timeval));


    while(FINISHED != MAX){
        
// STEP 1 ------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //checking for new connections
        //need to do sth about waiting time on 
        // client side.
        //MAKE SEPERATE METHOD FOR THIS
        //  THIS PART WORKS
        for(int i = 0; i < MAX; i++){
            if(clients[i] != NULL){
                switch(clients[i]->packet->flags){
                    case ACCEPT:
                        // rdp_printPacket(clients[i]->packet);
                        // send_packet(serverFd, (char*)clients[i]->packet, (sizeof(struct rdp_packet)+1000), 0, (struct sockaddr*)clients[i]->trgt_address, sizeof(struct sockaddr_in));
                        clients[i]->packet->flags = DATA;
                        // rdp_printPacket(clients[i]->packet);                        
                        break;
                }
            }
        }

// STEP 2 ------------------------------------------------------------------------------------------------------------------------------------------------------------------


        //prepare and sent next packet
        for(int i = 0; i < MAX; i++){
            if(clients[i] != NULL){
                
                int resend = 5; //tries to resend 5 times to get ack. otherwise it just continues 
                struct timeval replyTimeout;
                struct rdp_packet *replyPacket;


                printf("\n\nIF--------------------------- %d \n\n", i);
                rdp_printPacket(clients[i]->packet);

                if(clients[i]->packet->flags == TERMINATE) continue;

                while(resend){

                        //send -> works
                        FD_ZERO(&fds);

                        // printf("%d\n", resend);
                        // printf("STUCK\n");
                        // printf("%p\n", clients[i]->trgt_address);
                        
                        replyTimeout.tv_sec = 1;

                        FD_SET(serverFd, &fds);

                        //if previous packet acked, prepare for data retriev
                        if(clients[i]->packet->flags == ACK){
                            clients[i]->packet->flags = DATA;
                        }
                        if(clients[i]->packet->ackseq == clients[i]->packet->pktseq && clients[i]->packet->flags == DATA){
                            rdp_read(clients[i], clients[i]->packet); //will be 16 when eof has been reached                       
                            if((ntohs(clients[i]->packet->metadata) - sizeof(struct rdp_packet) == 0)) clients[i]->packet->metadata = htons(0); //makes sure termination sequence happens
                            if(clients[i]->packet->pktseq != 0){
                                rdp_idSwitch(clients[i]->packet); //corrects packet switching bug.
                            }
                                
                            rdp_bitwiseIncrement(&clients[i]->packet->pktseq);
                            
                        }
                        // # 1 -- SEND CURRENT PACKET TO CLIENT
                        send_packet(serverFd, (char*)clients[i]->packet, (sizeof(struct rdp_packet)+1000), 0, (struct sockaddr*)clients[i]->trgt_address, sizeof(struct sockaddr_in));
                        rdp_printPacket(clients[i]->packet);
                        // printf("hello i am here!\n");

                        
                        rc = select(FD_SETSIZE, &fds, NULL, NULL, &replyTimeout);

                        if(FD_ISSET(serverFd, &fds)){
                                
                                //where to free this?
                                struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
                                socklen_t addr_len = sizeof(struct sockaddr_in);

                                replyPacket = calloc(1, (sizeof(struct rdp_packet) + 1000));
                                rc = recvfrom(serverFd, replyPacket, (sizeof(struct rdp_packet) + 1000), 0,  (struct sockaddr*)addr, &addr_len); //activity stro

                                switch(replyPacket->flags){ //check flag SOURCE OF THE BUGSSSSSS NEED TO FREE

                                    // ALTERNATIVE 1 : REPLYPACKET IS AN ACKNOWLEDGMENT, STORE REPLY IN CURRENT CLIENT STRUCT AND STOP RESENDING! GO TO NEXT CLIENT IN LIST.
                                    case ACK: ;

                                        printf("REPLY ===================================\n");
                                        rdp_printPacket(replyPacket);
                                        // printf("%p\n", replyPacket);

                                        if(clients[i]->packet->flags == ACCEPT){ 
                                            //WEIRD BUG, FOR SOME REASON FD ISSET IS TRIGGERED AFTER SENDING 0X10 PACKET?
                                            //THE REPLY PACKET IS SAID TO BE AN ACK PACKET, BUT I CANNOT FIND OUT WHERE IT COMES FROM.... 
                                            //THE ACK PACKET IS THE SAME AS THE ACK PACKET OF THE PRVIOUS CLIENT
                                            //REPLYPACKET REFERENCE SHOULDN OT BE ABLE TO REFER BACK TO IT, UNLESS CLIENT SENDS IT TWICE???
                                            //WEIRD STUFF, BUT THIS IS A FIX AT LEAST......
                                            printf("BREAKING\n");
                                            resend = 0;
                                            break;
                                        }

                                        // STORING REPLY IN CLIENT STRUCT
                                        free(clients[i]->packet);
                                        free(addr);
                                        clients[i]->packet = replyPacket;
                                        
                                        resend = 0; //STOPPING RESEND

                                        //SPECIAL CONDITION FINAL PACKET. SET METADATA TO 0, SENT PACKAGE ON NEXT RESEND.
                                        if((ntohs(clients[i]->packet->metadata) - sizeof(struct rdp_packet)) < 900){
                                            printf("[NEXT PACKET LOOP] SENDING EMPTY PACKET\n"); //i interpreted an empty packet as packet with metadata = 0;
                                            
                                            clients[i]->packet->metadata = htons(0);

                                            //empty packet will be send on next resend                                            
                                            resend = 1;
                                        }

                                        break;

                                    
                                    // ALTERNATIVE 2 : REPLYPACKET IS A RANDOM CONNECT, CHECK VALIDITY REQUEST, ADD CLIENT AND RESEND PACKET OF CURRENT CLIENT ON NEXT LOOP TO AIT FOR ACK.
                                    case CONNECT: ;

                                        int idExists = 0;
                                        for(int j = 0; j < MAX; j++){
                                            if(clients[j] != NULL && clients[j]->packet->senderid == replyPacket->senderid){
                                                idExists = 1;
                                            }
                                        }

                                        struct rdp_connection *tmp = rdp_accept(replyPacket, &MAX, &CURRENT, addr, filename ,idExists);
                                        if(tmp == NULL){
                                            printf("[NEXT PACKET LOOP] NOT CONNECTED <%d> <%d> ---------------------------------------\n", ntohs(replyPacket->recvid), ntohs(replyPacket->senderid));
                                            send_packet(serverFd, (char*)replyPacket, (sizeof(struct rdp_packet)+1000), 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
                                            free(replyPacket);
                                            free(addr);
                                        } else{
                                            clients[CURRENT-1] = tmp;
                                            printf("[NEXT PACKET LOOP] CONNECTED <%d> <%d> ---------------------------------------\n", ntohs(tmp->packet->recvid), ntohs(tmp->packet->senderid));
                                            send_packet(serverFd, (char*)tmp->packet, (sizeof(struct rdp_packet)+1000), 0, (struct sockaddr*)tmp->trgt_address, sizeof(struct sockaddr_in)); //REPLY RECEIVED
                                            //PACKET WITH 0X10 SENT AND RECEIVED CONFIRMED

                                            rdp_printPacket(tmp->packet);
                                        }
                                    
                                        break;

                                    // ALTERNATIVE 3 : REPLYPACKET IS A TERMINATION - STORE REPLY IN CLIENT, PRINT MESSAGE, FREE MEMORY AND TERMINATE CONNECTION
                                     case TERMINATE:

                                        // free(clients[i]->packet);
                                        free(addr);
                                        free(replyPacket);
                                        // clients[i]->packet = replyPacket;
                                        printf("[NEXT PACKET LOOP] TERMINATION CONFIRMED\n");
                                        printf(" DISCONNECTED <CLIENT : %d> <SERVER: %d>\n", ntohs(clients[i]->packet->recvid), ntohs(clients[i]->packet->senderid)); //SERVER = SENDERID. ACK PACKETS ARE AUTOMATIVALLY CONVERTED TO DATA PACKETS WHEN RESEND IS ACTIVE. ID'S ARE SWITCH IN THE PROCESS
                                        rdp_close(clients[i]);
                                        clients[i] = NULL;
                                        FINISHED += 1;
                                        // rdp_printPacket(clients[i]->packet);
                                        resend = 0; //termination confirmed, end while loop
                                        break;
                                    
                                    default:
                                        free(addr);
                                        free(replyPacket);

                                } //SWITCH
                        
                        } //if fdisset
                        
                        if(clients[i] != NULL){
                            if(clients[i]->packet->flags == TERMINATE) resend = 0;
                            if(clients[i]->packet->flags == ACCEPT) resend = 0;
                        }

                }//while

            }//if

        }//for 


        int eventLoop = 1;

        while(eventLoop){
            FD_ZERO(&fds);

            eventTimeout.tv_usec = 500;

            FD_SET(serverFd, &fds);

            // printf("are we getting here?\n");
            rc = select(FD_SETSIZE, &fds, NULL, NULL, &eventTimeout);

            socklen_t addr_len = sizeof(struct sockaddr_in);

            //when swapping packets, free packet in connection and assing it a new value.
            if(FD_ISSET(serverFd, &fds)){

                struct rdp_packet *currentPacket = calloc(1, (sizeof(struct rdp_packet) + 1000));
                struct sockaddr_in *src_addr = malloc(sizeof(struct sockaddr_in));

                rc = recvfrom(serverFd, currentPacket, sizeof(*currentPacket), 0, (struct sockaddr*)src_addr, &addr_len);
                check_error(rc, "recvfrom");

                switch(currentPacket->flags){
                    case CONNECT: ;

                        int idExists = 0;
                        for(int i = 0; i < MAX; i ++){
                            if(clients[i] != NULL && clients[i]->packet->senderid == currentPacket->senderid){
                                idExists = 1;
                            }
                        }

                        struct rdp_connection *tmp = rdp_accept(currentPacket, &MAX, &CURRENT, src_addr, filename, idExists);
                        if(tmp == NULL){
                            printf("[RDP ACCEPT] NOT CONNECTED <%d> <%d> \n", ntohs(currentPacket->recvid), ntohs(currentPacket->senderid));
                            send_packet(serverFd, (char*)currentPacket, (sizeof(struct rdp_packet)+1000), 0, (struct sockaddr*)src_addr, sizeof(struct sockaddr_in));
                            free(currentPacket);
                            free(src_addr);
                        } else{
                            clients[CURRENT-1] = tmp;
                            printf("[RDP ACCEPT] CONNECTED <%d> <%d>\n", ntohs(tmp->packet->recvid), ntohs(tmp->packet->senderid));
                        }
                        break;
                    
                    }

                }
    
            

            if(rc == 0){
                // printf("NOTHING IS HAPPENING\n");
                eventLoop = 0;
            }
        }
    }

    printf("[SERVER] ALL FILES HAVE BEEN SERVED\nTERMINATING SERVER");
    free(clients);
    close(serverFd);
    return EXIT_SUCCESS;
}
