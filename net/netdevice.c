#include <net/netdevice.h>
#include <net/ether.h>
#include <net/arp.h>
#include <net/ip.h>
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
    switch (ntohs(eth->h_proto)) {
        case ETH_P_ARP:
            return arp_rcv(packet, size, dev);
        case ETH_P_IP:
            return ip_rcv(packet, size, dev);
    }
    return -1;
}