#include <net/net.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/ether.h>

int icmp_send(int type, int code, unsigned int dest_ip, struct net_device *dev, unsigned int src_ip, const unsigned char *src_hw, const unsigned char *target_hw)
{
    if (!src_hw) {
        src_hw = dev->dev_addr;
    }

    struct ethhdr *eth = kzmalloc(sizeof(struct ethhdr));
    if (dev_hard_header(eth, dev, ETH_P_IP, target_hw, src_hw)<0) {
        goto out;
    }

    struct iphdr *ip = kzmalloc(sizeof(struct iphdr));
    ip->version = 4;
    ip->id = 1;
    ip->protocol = IP_P_ICMP;
    ip->daddr = dest_ip;
    ip->saddr = src_ip;
    ip->ttl = 255;
    ip->ihl = sizeof(struct iphdr);
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);

    struct icmphdr *icmp = kzmalloc(sizeof(struct icmphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = 1;
    icmp->un.echo.sequence = 1;
    icmp->checksum = ip_hdr_checksum((unsigned short *)icmp, sizeof(struct icmphdr));

    ip->check=ip_hdr_checksum((unsigned short *)icmp, ip->tot_len);

    unsigned char *temp = kzmalloc(sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr));
    memcpy(temp, eth, sizeof(struct ethhdr));
    memcpy(temp + sizeof(struct ethhdr), ip, sizeof(struct iphdr));
    memcpy(temp + sizeof(struct ethhdr)+sizeof(struct iphdr), icmp, sizeof(struct icmphdr));

    dev->send(dev, temp, sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr));
    kfree(temp, sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr));
    kfree(ip, sizeof(struct iphdr));
    kfree(icmp, sizeof(struct icmphdr));
out:
    kfree(eth, sizeof(struct ethhdr));
    return 0;
}

int icmp_rcv(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct iphdr *ip = (struct iphdr*)((unsigned int)packet + sizeof(struct ethhdr));
    unsigned short body_offset = ntohs(ip->tot_len) - (unsigned short)ip->ihl; //ip数据部分开始位置
    if (ip_hdr_checksum((unsigned short*)ip, ip->ihl * 32/8)!=0) {
        return -1;
    }

    printk("v:%x hl:%x l:%x id:%x ttl:%x p:%x c:%x, sip:%x dip:%x\n", ip->version, ip->ihl, ip->tot_len, ip->id, ip->ttl, ip->protocol, ip->check, ip->saddr, ip->daddr);
    return -1;
}

static int icmp_echo(struct net_device *dev)
{
    struct ethhdr *eth = kmalloc(sizeof(struct ethhdr));
    if (dev_hard_header(eth, dev, ptype, dest_hw, src_hw)<0) {
        goto out;
    }

	struct icmphdr * icmp

	net = dev_net(skb_dst(skb)->dev);
	if (!net->ipv4.sysctl_icmp_echo_ignore_all) {
		struct icmp_bxm icmp_param;

		icmp_param.data.icmph	   = *icmp_hdr(skb);
		icmp_param.data.icmph.type = ICMP_ECHOREPLY;
		icmp_param.skb		   = skb;
		icmp_param.offset	   = 0;
		icmp_param.data_len	   = skb->len;
		icmp_param.head_len	   = sizeof(struct icmphdr);
		icmp_reply(&icmp_param, skb);
	}
	/* should there be an ICMP stat for ignored echos? */
	return 0;
}