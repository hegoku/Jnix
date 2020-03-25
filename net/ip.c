#include <net/net.h>
#include <net/ip.h>
#include <net/netdevice.h>
#include <net/icmp.h>
#include <stdio.h>

int ip_send(int type, unsigned int dest_ip, struct net_device *dev, unsigned int src_ip, const unsigned char *target_hw, unsigned char *data, unsigned short size)
{
    if (size>IPV4_MAX_LEN) {
        return -1;
    }
    return -1;
}

int ip_rcv(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct iphdr *ip = (struct iphdr*)((unsigned int)packet + sizeof(struct ethhdr));
    size -= sizeof(struct iphdr);

    switch (ntohs(ip->protocol))
    {
        case IP_P_ICMP:
            return icmp_rcv(packet, size, dev);
        case IP_P_TCP:
            break;
        case IP_P_UDP:
            break;
    }
    return -1;
}

unsigned short ip_hdr_checksum(unsigned short* buffer, int size)
{
    unsigned int cksum = 0;
    while(size>1)
    {
        cksum += htons(*buffer++);
        size -= sizeof(unsigned short);
        printk("%x ", cksum);
    }
    if(size)
    {
        cksum += *(unsigned char*)buffer;
    }
    printk("\n");
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum>>16); 
    return (unsigned short)(~cksum);
}