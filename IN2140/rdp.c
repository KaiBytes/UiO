/**
    !! RDP uses one single UDP port to receive packets from all clients

    1. client needs to be able to request connection
    2. server needs to be able to check new requests
    - accepted? -> server responds with packet with flag == 0x10 && prints connection ID's of server and client
    - current file == N? --> denies connection && prints NOT CONNECTED <client-ID> <server-ID>
    ??? how to check if new request has arrived ???
    ??? how to connect rdp struct to an address ???
        rdp protocol meeting place for server and client
        rdp protocol has own socket whcih will listen for connection requests
        client can call rdp request function.
            client will generate random id and sent as message to rdp server
        rdp server will 
        the addresses of clients will be stored in a list called new connections
        rdp_accept function will connect clients to the server as long as fewer than N files have been serviced
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rdp.h"
#include "send_packet.h"


void check_error(int res, char *msg){
    if(res == -1){
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

void rdp_bitwiseIncrement(unsigned char *id){
    unsigned char oneMask = 0b00000001;
    while(*id & oneMask){
        *id ^= oneMask;
        oneMask <<= 1;
    }
    *id ^= oneMask;
}


FILE* rdp_openFile(char *fileName){
    FILE *f;
    f = fopen(fileName, "rb");
    if (!f) {
        perror("fopen");
        fprintf(stderr, "[RDP_OPENFILE] UNABLE TO OPEN FILE! CHECK IF FILENAME IS SPELLED CORRECTLY\n");
        exit(EXIT_FAILURE);
    }
    return f;
}

void rdp_idSwitch(struct rdp_packet *packet){
    if(packet->senderid != 0){
        packet->recvid = packet->senderid;
        packet->senderid = 0;
    } else if (packet->senderid == 0){
        packet->senderid = packet->recvid;
        packet->recvid = 0;
    }
}

//returns fd = -1 on failed connection, otherwise returns socket fd
int rdp_connect(struct sockaddr *address, socklen_t sockLen, struct rdp_packet *connectPacket){
    
    int fd, wc, rc;
    fd_set fds;

    struct timeval timeout;
    //dealing with valgrind
    memset(&timeout, 0, sizeof(struct timeval)); 

    //creating socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(fd, "socket");

    //sending packet
    wc = send_packet(fd, (char *)connectPacket, sizeof(struct rdp_packet), 0, address, sockLen);
    check_error(wc, "send_packet");

    rdp_printPacket(connectPacket);

    //waiting for confirmation. waits for 1 sec. no reaction? return fd = -1
    while(fd){

        FD_ZERO(&fds);

        //wait for 1 sec. no reaction? return -1
        timeout.tv_sec = 1;

        FD_SET(fd, &fds);

        rc = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);

        if(FD_ISSET(fd, &fds)){

            rc = recv(fd, connectPacket, sizeof(struct rdp_packet) ,0);
            check_error(rc, "recv");

            switch(connectPacket->flags){
                case ACCEPT:
                    printf(" [RDP ACCEPT] CONNECTED <%d> <%d> ---------------------------------------\n", ntohs(connectPacket->recvid), ntohs(connectPacket->senderid));
                    break;
                case REFUSE:
                    printf("[RDP ACCEPT] NOT CONNECTED <%d> <%d> ---------------------------------------\n", ntohs(connectPacket->recvid), ntohs(connectPacket->senderid));
                    close(fd);

                    if(ntohs(connectPacket->metadata) == 950){
                        printf("[RDP ACCEPT] SERVER IS AT MAX CAPACITY! UNABLE TO CONNECT\n");
                    }

                    if(ntohs(connectPacket->metadata) == 960){
                        printf("[RDP ACCEPT] CLIENT WITH THAT ID ALREADY EXISTS\n");
                    }

                    fd = -1;
                    break;
                default:
                    printf("[RDP_CONNECT] INVALID FLAG");
            }

            break; 
        }

        if(rc == 0){
            fprintf(stderr, "*** [RDP_CONNECT] NO CONNECTION COULD BE MADE ***\n");
            close(fd);
            fd = 0;
        }       
    }
    return fd;
}

struct rdp_connection* rdp_accept(struct rdp_packet *packet, int *maxCapacity, int *currentCapacity, struct sockaddr_in *src_addr, char* fName, int idExists){

    rdp_idSwitch(packet);

    if(packet->flags == 0x01 && *currentCapacity == *maxCapacity){
        
        packet->flags = 0x20;
        packet->metadata = htons(950);
        return NULL;

    } else {

        if(idExists){
            
            packet->flags = 0x20;
            packet->metadata = htons(960);

            return NULL;
        } else{

            // printf("*** CONNECT REQUEST ACCEPTED***\n");
            packet->flags = 0x10;
            struct rdp_connection *newClient = malloc(sizeof(struct rdp_connection));
            newClient->trgt_address = src_addr;
            newClient->packet = packet;
            newClient->f = rdp_openFile(fName);
            
            *currentCapacity += 1;

            return newClient;
        }

    }

}

//when server prepares ox40 packet, run this method. packet has initialized all of its memory @server
int rdp_read(struct rdp_connection *client, struct rdp_packet *packet){
    
    int rc, total;

    rc = fread(packet->payload, sizeof(char), 900, client->f);
    if(rc  == 0){
        perror("fread");
        fprintf(stderr, "could not read");
    }

    // printf("%s\n", packet->payload);
    total = rc + sizeof(struct rdp_packet);
    packet->metadata = htons(total);    
    return rc;
}

void rdp_write(struct rdp_packet *packet, FILE *f){
    
    int rc;

    // printf("file %p\n", f);

    if((rc = fwrite(packet->payload, sizeof(char),ntohs(packet->metadata) - sizeof(struct rdp_packet),f)) == 0){
        perror("fwrite");
        fprintf(stderr, "[RDP_WRITE] COULD NOT WRITE");
        exit(EXIT_FAILURE);
    }    

}

//needed for closing connections
void rdp_close(struct rdp_connection *connection){
    free(connection->trgt_address);
    fclose(connection->f);
    free(connection->packet);
    free(connection);
}

void rdp_printPacket(struct rdp_packet *currentPacket){
    printf("flag: %x\n", currentPacket->flags);
    printf("pktseq: %d\n", currentPacket->pktseq);
    printf("ackseq: %d\n", currentPacket->ackseq);
    printf("unassigned: %c\n", currentPacket->unassigned);
    printf("senderId: %d\n", ntohs(currentPacket->senderid));
    printf("recvId: %d\n", ntohs(currentPacket->recvid));
    printf("metadata: %d\n", ntohs(currentPacket->metadata));
}


