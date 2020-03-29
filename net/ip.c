#include <net/net.h>
#include <net/ip.h>
#include <net/netdevice.h>
#include <net/icmp.h>
#include <stdio.h>
#include <net/arp.h>
#include <string.h>
#include <system/mm.h>
#include <ctype.h>

int ip_send(int type, unsigned short id, unsigned int dest_ip, unsigned int src_ip, struct net_device *dev, unsigned char *data, unsigned short size)
{
    if (size>IPV4_MAX_LEN) {
        return -1;
    }

    unsigned char *dest_hw = 0;
    if (!dest_ip)
    { //broadcast
        dest_ip = 0xFFFFFFFF;
        unsigned char boardcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        dest_hw = boardcast;
    }
    else
    {
        dest_hw=arp_find(dev, dest_ip);
        // unsigned char null_mac[]={'0', '0', '0', '0', '0', '0'};
        // int a=dest_hw[0]+dest_hw[1]+dest_hw[2]+dest_hw[3]+dest_hw[4]+dest_hw[5];
        if (dest_hw==0) {
            printk("unkown ip: %x\n", dest_ip);
            return -1;
        }
    }
    
    
    unsigned int buffer_size = sizeof(struct ethhdr) + sizeof(struct iphdr) + size;
    unsigned char *buffer=kzmalloc(buffer_size);
    struct ethhdr *eth = (struct ethhdr*)buffer;
    if (dev_hard_header(eth, dev, ETH_P_IP, dest_hw, dev->dev_addr)<0) {
        goto out;
    }

    struct iphdr *ip=(struct iphdr *)((unsigned int)buffer+sizeof(struct ethhdr));
    ip->daddr=htonl(dest_ip);
    ip->saddr=htonl(src_ip);
    ip->protocol=type;
    ip->version=4;
    ip->ihl=sizeof(struct iphdr)/4;
    ip->tot_len=htons(sizeof(struct iphdr)+size);
    ip->ttl=0x40;
    ip->id=htons(id);

    memcpy((void*)((unsigned int)ip+sizeof(struct iphdr)), data, size);
    
    ip->check=htons(ip_hdr_checksum((unsigned short*)ip, sizeof(struct iphdr)+size));

    return dev->send(dev, buffer, buffer_size);
out:
    kfree(buffer, buffer_size);
    return -1;
}

int ip_rcv(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct iphdr *ip = (struct iphdr*)((unsigned int)packet + sizeof(struct ethhdr));
    size -= sizeof(struct ethhdr);

    if (ip_hdr_checksum((unsigned short*)ip, size)!=0) {
        return -1;
    }

    printk("recv: ");
    for(int i = 0; i < size; i++)
    {
        printk("%02x ", packet[i]);
    }
    printk("\n");

    switch (ip->protocol)
    {
        case IP_P_ICMP:
            return icmp_rcv((unsigned char*)ip, size, dev);
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
    }
    if(size)
    {
        cksum += *(unsigned char*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum>>16);
    return (unsigned short)(~cksum);
}

int inet_aton(const char *cp)
{
	unsigned int val, base, n;
	char c;
	unsigned int parts[4], *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, other=decimal.
		 */
		val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while ((c = *cp) != '\0') {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}
			if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) + 
					(c + 10 - (islower(c) ? 'a' : 'A'));
				cp++;
				continue;
			}
			break;
		}
		if (*cp == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16-bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return (0);
			*pp++ = val, cp++;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && (!isascii(*cp) || !isspace(*cp)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
    return val;
}
