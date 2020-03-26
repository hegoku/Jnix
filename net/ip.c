#include <net/net.h>
#include <net/ip.h>
#include <net/netdevice.h>
#include <net/icmp.h>
#include <stdio.h>
#include <net/arp.h>
#include <string.h>
#include <system/mm.h>

int ip_send(int type, unsigned short id, unsigned int dest_ip, unsigned int src_ip, struct net_device *dev, unsigned char *data, unsigned short size)
{
    if (size>IPV4_MAX_LEN) {
        return -1;
    }

    unsigned char *dest_hw=arp_find(dev, dest_ip);
    unsigned char null_mac[]={'0', '0', '0', '0', '0', '0'};
    int a=dest_hw[0]+dest_hw[1]+dest_hw[2]+dest_hw[3]+dest_hw[4]+dest_hw[5];
    if (a==0) {
        printk("unkown ip: %d.%d.%d.%d\n", dest_ip);
        return -1;
    }


    unsigned int buffer_size=sizeof(struct ethhdr)+sizeof(struct iphdr)+size;
    unsigned char *buffer=kzmalloc(buffer_size);
    struct ethhdr *eth = (struct ethhdr*)buffer;
    if (dev_hard_header(eth, dev, ETH_P_IP, dest_hw, dev->dev_addr)<0) {
        goto out;
    }

    struct iphdr *ip=(struct iphdr *)((unsigned int)buffer+sizeof(struct ethhdr));
    ip->daddr=dest_ip;
    ip->saddr=src_ip;
    ip->protocol=type;
    ip->version=4;
    ip->ihl=sizeof(struct iphdr)/32;
    ip->tot_len=htons(ip->ihl+size);
    ip->ttl=0x40;
    ip->id=htons(id);
    ip->check=htons(ip_hdr_checksum((unsigned short*)ip, ip->tot_len));

    return dev->send(dev, buffer, buffer_size);
out:
    kfree(buffer, buffer_size);
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