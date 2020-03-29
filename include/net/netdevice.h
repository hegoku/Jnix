#ifndef _NET_NETDEVICE_H
#define _NET_NETDEVICE_H

#include <net/ether.h>

struct net_device {
    char name[256];

    unsigned char dev_addr[6];
    unsigned int ip;
    unsigned int mask;

    int irq;

    unsigned int io_base;

    unsigned int rx_dropped;
    unsigned int tx_dropped;

    void *custom_data;

    // int (*header_ops_create)(const unsigned char *buffer, struct net_device *dev, unsigned short type, const void *daddr, const void *saddr, unsigned int len);
    int (*header_ops_create)(struct ethhdr *eth, struct net_device *dev, unsigned short type, const void *daddr, const void *saddr);
    int (*send)(struct net_device *dev, unsigned char *packet, int size);

    int (*init)(struct net_device *dev);
};

int register_netdev(struct net_device *dev);

int reveice_callback(unsigned char *packet, unsigned int size, struct net_device *dev);

// static inline int dev_hard_header(unsigned char *buff, struct net_device *dev, unsigned short type, const void *daddr, const void *saddr, unsigned int len)
static inline int dev_hard_header(struct ethhdr *eth, struct net_device *dev, unsigned short type, const void *daddr, const void *saddr)
{
    if (!dev->header_ops_create) {
        return 0;
    }
    return dev->header_ops_create(eth, dev, type, daddr, saddr);
}

#endif