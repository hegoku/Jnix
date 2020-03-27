#ifndef _DRIVER_I386_NET_AM79C79C_H
#define _DRIVER_I386_NET_AM79C79C_H

#include <net/netdevice.h>

#define AM79C79C_MAC0_PORT 0x0
#define AM79C79C_MAC2_PORT 0x2
#define AM79C79C_MAC4_PORT 0x4
#define AM79C79C_DATA_PORT 0x10
#define AM79C79C_ADD_PORT 0x12
#define AM79C79C_RESET_PORT 0x14
#define AM79C79C_BUSREG_PORT 0x16

struct initialization_block {
    unsigned short mode;
    // unsigned char reserved1 : 4;
    // unsigned char rlen : 4;
    // unsigned char reserved2 : 4;
    // unsigned char tlen : 4;
    unsigned char rlen;
    unsigned char tlen;
    unsigned char mac[6];
    unsigned short reserved3;
    unsigned int ladr_low;
    unsigned int ladr_hight;
    unsigned int recvBufferDescrAddress;
    unsigned int sendBufferDescrAddress;
} __attribute__((packed));

struct buffer_descriptor {
        void* address;
        unsigned int flags;
        unsigned int flags2;
        unsigned int avail;
} __attribute__((packed));

struct am79c793_data
{
    struct initialization_block init_block;
    struct buffer_descriptor *sendBufferDescr;
    struct buffer_descriptor *recvBufferDescr;

    unsigned int current_rx_buffer;
    unsigned int current_tx_buffer;
};

#define TO_AMDATA(pointer) ((struct am79c793_data*)(pointer))

int send(struct net_device *dev, unsigned char *packet, int size);

extern struct net_device am79c793;

#endif