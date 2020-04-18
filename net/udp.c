#include <net/udp.h>
#include <net/net.h>
#include <net/netdevice.h>
#include <net/ip.h>
#include <system/mm.h>
#include <stdio.h>
#include <string.h>

int udp_send(struct net_device *dev, unsigned int dest_ip, unsigned int dest_port, unsigned int src_port, unsigned char *data, unsigned int data_size)
{
    unsigned int size = data_size + sizeof(struct udphdr)+sizeof(struct udpfakehdr);
    struct udpfakehdr *fake = kzmalloc(size);
    struct udphdr *udp = (struct udphdr*)((unsigned int)fake + sizeof(struct udpfakehdr));
    fake->dest_ip = htonl(dest_ip);
    fake->src_ip = htonl(dev->ip);
    fake->protocol = IP_P_UDP;
    fake->len = htons(size-sizeof(struct udpfakehdr));
    udp->source = htons(src_port);
    udp->dest = htons(dest_port);
    udp->len = htons(size-sizeof(struct udpfakehdr));
    memcpy((unsigned char*)((unsigned int)udp + sizeof(struct udphdr)), data, data_size);
    udp->check=htons(ip_hdr_checksum((unsigned short*)fake, size));

    ip_send(IP_P_UDP, 1, dest_ip, dev->ip, dev, (unsigned char*)udp, size-sizeof(struct udpfakehdr));
    kfree(fake, size);
    return 0;
}

int udp_rcv(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct iphdr *ip = (struct iphdr*)((unsigned int)packet);
    unsigned short body_offset = (unsigned short)ip->ihl*4; //ip数据部分开始位置
    struct udphdr *udp=(struct udphdr*)((unsigned int)ip+body_offset);
    // size -= sizeof(struct iphdr);
    struct udpfakehdr *fake = kzmalloc(size + sizeof(struct udpfakehdr));
    memcpy((unsigned char*)((unsigned int)fake + sizeof(struct udpfakehdr)), udp, size);
    fake->dest_ip = ip->daddr;
    fake->src_ip = ip->saddr;
    fake->protocol = IP_P_UDP;
    fake->len = ntohs(size);
    if (ip_hdr_checksum((unsigned short*)fake, size + sizeof(struct udpfakehdr))!=0) {
        kfree(fake, size + sizeof(struct udpfakehdr));
        return -1;
    }
    kfree(fake, size + sizeof(struct udpfakehdr));

    unsigned char *data_body = (unsigned char *)((unsigned int)udp + sizeof(struct udphdr));
    unsigned int data_body_size = size - sizeof(struct udphdr);

    printk("d_p:%x s_p:%x len:%x d:%x\n", udp->dest, udp->source, udp->len, size);

    udp_send(dev, ntohl(ip->saddr), ntohs(udp->source), ntohs(udp->dest), data_body, data_body_size);

    // printk("v:%x hl:%x l:%x id:%x ttl:%x p:%x c:%x, sip:%x dip:%x\n", ip->version, ip->ihl, ip->tot_len, ip->id, ip->ttl, ip->protocol, ip->check, ip->saddr, ip->daddr);
    return -1;
}