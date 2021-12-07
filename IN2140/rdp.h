struct rdp_packet {
    unsigned char flags;
    unsigned char pktseq;
    unsigned char ackseq;
    unsigned char unassigned;
    int senderid;
    int recvid;
    //metadata description vague, ask question about this
    int metadata;
    char payload[]; //8 bytes on 64 bit system.
    //24 bytes in this setup
}__attribute__((packed));

struct rdp_connection {
    struct sockaddr_in *trgt_address;
    FILE *f;
    struct rdp_packet *packet;
};

enum type {
    CONNECT = 0x01,
    TERMINATE = 0x02,
    DATA = 0x04,
    ACK = 0x08,
    ACCEPT = 0x10,
    REFUSE = 0x20,
};

void check_error(int res, char *msg);
int rdp_connect(struct sockaddr *address, __socklen_t sockLen, struct rdp_packet *connectPacket);
struct rdp_connection* rdp_accept(struct rdp_packet *packet, int *maxCapacity, int *currentCapacity, struct sockaddr_in *src_addr, char* fName, int idExists);
void rdp_printPacket(struct rdp_packet *currentPacket);
int rdp_read(struct rdp_connection *client, struct rdp_packet *packet);
void rdp_write(struct rdp_packet *packet, FILE *f);
void rdp_bitwiseIncrement(unsigned char *id);
void rdp_close(struct rdp_connection *connection);
void rdp_idSwitch(struct rdp_packet *packet);

