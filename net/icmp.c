#include <net/net.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/ether.h>
#include <string.h>
#include <system/mm.h>
#include <stdio.h>

int icmp_send(int type, int code, unsigned int dest_ip, struct net_device *dev, unsigned int src_ip, const unsigned char *src_hw, const unsigned char *target_hw)
{
    if (!src_hw) {
        src_hw = dev->dev_addr;
    }

//     struct ethhdr *eth = kzmalloc(sizeof(struct ethhdr));
//     if (dev_hard_header(eth, dev, ETH_P_IP, target_hw, src_hw)<0) {
//         goto out;
//     }

//     struct iphdr *ip = kzmalloc(sizeof(struct iphdr));
//     ip->version = 4;
//     ip->id = 1;
//     ip->protocol = IP_P_ICMP;
//     ip->daddr = dest_ip;
//     ip->saddr = src_ip;
//     ip->ttl = 255;
//     ip->ihl = sizeof(struct iphdr);
//     ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);

//     struct icmphdr *icmp = kzmalloc(sizeof(struct icmphdr));
//     icmp->type = ICMP_ECHO;
//     icmp->code = 0;
//     icmp->un.echo.id = 1;
//     icmp->un.echo.sequence = 1;
//     icmp->checksum = ip_hdr_checksum((unsigned short *)icmp, sizeof(struct icmphdr));

//     ip->check=ip_hdr_checksum((unsigned short *)icmp, ip->tot_len);

//     unsigned char *temp = kzmalloc(sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr));
//     memcpy(temp, eth, sizeof(struct ethhdr));
//     memcpy(temp + sizeof(struct ethhdr), ip, sizeof(struct iphdr));
//     memcpy(temp + sizeof(struct ethhdr)+sizeof(struct iphdr), icmp, sizeof(struct icmphdr));

//     dev->send(dev, temp, sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr));
//     kfree(temp, sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr));
//     kfree(ip, sizeof(struct iphdr));
//     kfree(icmp, sizeof(struct icmphdr));
// out:
    // kfree(eth, sizeof(struct ethhdr));
    return 0;
}

int icmp_rcv(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct iphdr *ip = (struct iphdr*)((unsigned int)packet);
    unsigned short body_offset = (unsigned short)ip->ihl*4; //ip数据部分开始位置
    struct icmphdr *icmp=(struct icmphdr*)((unsigned int)ip+body_offset);
    // size -= sizeof(struct iphdr);
    if (ip_hdr_checksum((unsigned short*)icmp, size)!=0) {
        return -1;
    }

    unsigned char *data_body = (unsigned char *)((unsigned int)icmp + sizeof(struct icmphdr));
    unsigned int data_body_size = size - sizeof(struct icmphdr);

    printk("type:%x code:%x id:%x s:%x\n", icmp->type, icmp->code, icmp->un.echo.id, icmp->un.echo.sequence);

    switch (icmp->type)
    {
    case ICMP_ECHO:
        icmp_echoreply(dev, ntohs(icmp->un.echo.id), ntohs(icmp->un.echo.sequence), ntohl(ip->saddr),  data_body, data_body_size);
        break;
    case ICMP_ECHOREPLY:
        break;
    case ICMP_ADDRESSREPLY:
        printk("mask: %x, from %x\n", (unsigned int)(*data_body), ntohl(ip->saddr));
        break;
    case ICMP_TIMESTAMPREPLY: {
            struct icmp_timestamp *time= data_body;
            printk("origin: %u, recv:%u, xmit:%u\n", ntohl(time->orig), ntohl(time->recv), ntohl(time->xmit));
            break;
        }
    case ICMP_NET_ANO: {
        struct icmp_route *route = (struct icmp_route *)data_body;
        printk("route_count: %u, route_len:%u:\n", icmp->un.route.count, icmp->un.route.len);
        for (int i = 0; i < icmp->un.route.count;i++) {
            printk("route: %x, p: %u\n", route[i].ip, route[i].p);
        }
            break;
    }
        
    }

    // printk("v:%x hl:%x l:%x id:%x ttl:%x p:%x c:%x, sip:%x dip:%x\n", ip->version, ip->ihl, ip->tot_len, ip->id, ip->ttl, ip->protocol, ip->check, ip->saddr, ip->daddr);
    return -1;
}

int icmp_echo(struct net_device *dev, unsigned short id, unsigned short sequence, unsigned int dest_ip)
{
	struct icmphdr *icmp=kzmalloc(sizeof(struct icmphdr));
    icmp->type=ICMP_ECHO;
    icmp->code=0;
    icmp->un.echo.id=htons(id);
    icmp->un.echo.sequence=htons(sequence);
    icmp->checksum=htons(ip_hdr_checksum((unsigned short*)icmp, sizeof(struct icmphdr)));

    ip_send(IP_P_ICMP, sequence, dest_ip, dev->ip, dev, (unsigned char*)icmp, sizeof(struct icmphdr));
    kfree(icmp, sizeof(struct icmphdr));
	return 0;
}

int icmp_echoreply(struct net_device *dev, unsigned short id, unsigned short sequence, unsigned int dest_ip, unsigned char *data, unsigned int data_size)
{
    unsigned int total_size = sizeof(struct icmphdr) + data_size;
    struct icmphdr *icmp=kzmalloc(total_size);
    icmp->type = ICMP_ECHOREPLY;
    icmp->code=0;
    icmp->un.echo.id=htons(id);
    icmp->un.echo.sequence=htons(sequence);
    memcpy((unsigned char*)((unsigned int)icmp + sizeof(struct icmphdr)), data, data_size);
    icmp->checksum = htons(ip_hdr_checksum((unsigned short *)icmp, total_size));

    ip_send(IP_P_ICMP, sequence, dest_ip, dev->ip, dev, (unsigned char*)icmp, total_size);
    kfree(icmp, total_size);
	return 0;
}

int icmp_address(struct net_device *dev, unsigned short id, unsigned short sequence, unsigned int dest_ip)
{
    unsigned int size = sizeof(struct icmphdr) + sizeof(unsigned int);
    struct icmphdr *icmp = kzmalloc(size);
    icmp->type=ICMP_ADDRESS;
    icmp->code=0;
    icmp->un.echo.id=htons(id);
    icmp->un.echo.sequence=htons(sequence);
    unsigned int *mask_offset= (unsigned int)icmp + sizeof(struct icmphdr);
    *mask_offset = 0;
    icmp->checksum=htons(ip_hdr_checksum((unsigned short*)icmp, size));

    ip_send(IP_P_ICMP, sequence, dest_ip, dev->ip, dev, (unsigned char*)icmp, size);
    kfree(icmp, size);
	return 0;
}

int icmp_timestamp(struct net_device *dev, unsigned short id, unsigned short sequence, unsigned int dest_ip, unsigned int origin)
{
    unsigned int size = sizeof(struct icmphdr) + sizeof(unsigned int)*3;
    struct icmphdr *icmp = kzmalloc(size);
    icmp->type=ICMP_TIMESTAMP;
    icmp->code=0;
    icmp->un.echo.id=htons(id);
    icmp->un.echo.sequence=htons(sequence);
    struct icmp_timestamp *body = (struct icmp_timestamp*)((unsigned int)icmp + sizeof(struct icmphdr));
    body->orig = htonl(origin);
    icmp->checksum = htons(ip_hdr_checksum((unsigned short *)icmp, size));

    ip_send(IP_P_ICMP, sequence, dest_ip, dev->ip, dev, (unsigned char*)icmp, size);
    kfree(icmp, size);
	return 0;
}

int icmp_hostano(struct net_device *dev, unsigned int dest_ip)
{
	struct icmphdr *icmp=kzmalloc(sizeof(struct icmphdr));
    icmp->type=ICMP_HOST_ANO;
    icmp->code=0;
    icmp->checksum=htons(ip_hdr_checksum((unsigned short*)icmp, sizeof(struct icmphdr)));

    ip_send(IP_P_ICMP, 1, dest_ip, dev->ip, dev, (unsigned char*)icmp, sizeof(struct icmphdr));
    kfree(icmp, sizeof(struct icmphdr));
	return 0;
}