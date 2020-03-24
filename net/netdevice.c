#include <net/netdevice.h>
#include <net/ether.h>
#include <net/arp.h>
#include <net/net.h>
#include <stdio.h>

int register_netdev(struct net_device *dev)
{
    if (dev->init) {
        return dev->init(dev);
    }
    return 0;
}

int reveice_callback(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct ethhdr *eth = (struct ethhdr*)packet;
    if (eth->h_proto == htons(ETH_P_ARP))
    {
        return arp_rcv(packet, size, dev);
    }
    return -1;
}