#include <net/arp.h>
#include <net/net.h>
#include <net/ether.h>
#include <net/netdevice.h>
#include <system/mm.h>
#include <string.h>
#include <stdio.h>

struct hash_entry {
    unsigned int ip;
    unsigned char mac[6];
    struct hash_entry *next;
};

struct hash_map{
    int size;
    int list_size;
    struct hash_entry *list;
} * arp_map=0;

int hash_code(struct hash_map *map, unsigned int ip) {
    // char *k = (char *)&ip;
    // unsigned int h = 0;
    // for (int i = 0; i < 6;i++)
    // {
    //     h = (h << 4) + k[i];
    //     unsigned int g = h & 0xF0000000;
    //     if (g) {
    //         h ^= g >> 24;
    //     }
    //     h &= ~g;
    // }
    // printk("ip:%x index:%x\n", ip, h % map->list_size);
    return ip % map->list_size;
}

void hash_put(struct hash_map *map, unsigned int ip, unsigned char* mac)
{
    int index = hash_code(map, ip);
    if (map->list[index].ip == 0)
    {
        map->size++;
        map->list[index].ip = ip;
        memcpy(map->list[index].mac, mac, 6);
    }
    else
    {
        struct hash_entry *current = &map->list[index];
        while (current!=0) {
            if (current->ip==ip) {
                memcpy(map->list[index].mac, mac, 6);
                return;
            }
            current = current->next;
        }

        struct hash_entry *n = kzmalloc(sizeof(struct hash_entry));
        n->ip = ip;
        memcpy(n->mac, mac, 6);
        n->next = map->list[index].next;
        map->list[index].next = n;
        map->size++;
    }
}

unsigned char* hash_get(struct hash_map *map, unsigned int ip)
{
    int index = hash_code(map, ip);
    struct hash_entry *e = &(map->list[index]);
    while(e->ip!=0 && ip!=e->ip) {
        e = e->next;
    }
    return e->mac;
}

void hash_remove(struct hash_map *map, unsigned int ip) {
    int index = hash_code(map, ip);
    struct hash_entry *entry = &(map->list[index]);
    if (entry->ip == 0) {
        return;
    }
    if (entry->ip==ip) {
        map->size--;
        if (entry->next != 0) {
            struct hash_entry *temp = entry->next;
            entry->ip = temp->ip;
            memcpy(entry->mac, temp->mac, 6);
            entry->next = temp->next;
            kfree(temp, sizeof(struct hash_entry));
        }
        else {
            entry->ip = 0;
            memset(entry->mac, 0, 6);
        }
    }
    else {
        struct hash_entry *p = entry;
        entry = entry->next;
        while (entry != 0) {
            if (entry->ip==ip) {
                map->size--;
                p->next = entry->next;
                kfree(entry, sizeof(struct hash_entry));
                break;
            }
            p = entry;
            entry = entry->next;
        };
    }
}

void hash_clear(struct hash_map *map) {
    memset(arp_map->list, 0, sizeof(struct hash_entry)*10);
    map->list = 0;
    map->size = 0;
}

int arp_send(int type, int ptype, unsigned int dest_ip, struct net_device *dev, unsigned int src_ip, const unsigned char *dest_hw, const unsigned char *src_hw, const unsigned char *target_hw)
{
    if (!src_hw) {
        src_hw = dev->dev_addr;
    }
    if (!dest_hw) {
        unsigned char boardcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        dest_hw = boardcast;
    }

    struct ethhdr *eth = kzmalloc(sizeof(struct ethhdr));
    if (dev_hard_header(eth, dev, ptype, dest_hw, src_hw)<0) {
        goto out;
    }

    struct arphdr *arp = kzmalloc(sizeof(struct arphdr));
    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ETH_P_IP);
    arp->ar_hln = 6;
    arp->ar_pln = 4;
    arp->ar_op = htons(type);

    memcpy(arp->ar_sha, src_hw, sizeof(arp->ar_sha));
    memcpy(arp->ar_sip, &src_ip, 4);

    if (target_hw) {
        memcpy(arp->ar_tha, target_hw, sizeof(arp->ar_tha));
    } else {
        memset(arp->ar_tha, 0, sizeof(arp->ar_tha));
    }
    memcpy(arp->ar_tip, &dest_ip, 4);

    unsigned char *temp = kzmalloc(sizeof(struct ethhdr)+sizeof(struct arphdr));
    memcpy(temp, eth, sizeof(struct ethhdr));
    memcpy(temp + sizeof(struct ethhdr), arp, sizeof(struct arphdr));

    dev->send(dev, temp, sizeof(struct ethhdr)+sizeof(struct arphdr));
    kfree(temp, sizeof(struct ethhdr) + sizeof(struct arphdr));
    kfree(arp, sizeof(struct arphdr));
out:
    kfree(eth, sizeof(struct ethhdr));
    return 0;
}

int arp_rcv(unsigned char *packet, unsigned int size, struct net_device *dev)
{
    struct ethhdr *eth = (struct ethhdr *)packet;
    struct arphdr *arp = (struct arphdr*)((unsigned int)packet + sizeof(struct ethhdr));
    size -= sizeof(struct ethhdr);

    if (arp->ar_hrd!=htons(ARPHRD_ETHER)) {
        return -1;
    }
    if (arp->ar_op!=htons(ARPOP_REPLY) && arp->ar_op!=htons(ARPOP_REQUEST)) {
        return -1;
    }

    unsigned int sip;
    memcpy(&sip, arp->ar_sip, sizeof(arp->ar_sip));
    unsigned int myip = 0x0a00020C;
    if (arp->ar_op == htons(ARPOP_REQUEST))
    {
        unsigned int tip;
        memcpy(&tip, arp->ar_tip, sizeof(arp->ar_tip));
        if (tip == htonl(myip))
        {
            arp_send(ARPOP_REPLY, ETH_P_ARP, sip, dev, htonl(myip), arp->ar_sha, dev->dev_addr, arp->ar_sha);
        }
    }
    else if (arp->ar_op == htons(ARPOP_REPLY))
    {
        if (arp_map==0) {
            arp_map = (struct hash_map*)kzmalloc(sizeof(struct hash_map));
            arp_map->list = (struct hash_entry*)kzmalloc(sizeof(struct hash_entry)*10);
            arp_map->list_size = 10;
            arp_map->size = 0;
        }
        hash_put(arp_map, sip, arp->ar_sha);
        unsigned char *th_m = hash_get(arp_map, sip);
        printk("mac:%02X %02X %02X %02X %02X %02X , ip:%x\n", th_m[0], th_m[1], th_m[2], th_m[3], th_m[4], th_m[5], sip);
    }
}

unsigned char *arp_find(struct net_device *dev, unsigned int ip)
{
    if (arp_map==0) {
        arp_map = (struct hash_map*)kzmalloc(sizeof(struct hash_map));
        arp_map->list = (struct hash_entry*)kzmalloc(sizeof(struct hash_entry)*10);
        arp_map->list_size = 10;
        arp_map->size = 0;
    }
    unsigned char *mac=hash_get(arp_map, ip);
    unsigned char null_mac[]={'0', '0', '0', '0', '0', '0'};
    int a=mac[0]+mac[1]+mac[2]+mac[3]+mac[4]+mac[5];
    if (a==0) {
        arp_send(ARPOP_REQUEST, ETH_P_ARP, htonl(ip), dev, htonl(dev->ip), 0, 0, 0);
        return hash_get(arp_map, ip);
    } else {
        return mac;
    }
}

// struct skb *arp_create(int type, int ptype, unsigned int dest_ip, struct net_device *dev, unsigned int src_ip, const unsigned char *dest_hw, const unsigned char *src_hw, const unsigned char *target_hw)
// {
//     struct skb *skb=kzmalloc(sizeof(skb));
//     unsigned char *buffer = kzmalloc(2048);
//     skb->data = buffer;
//     skb->len = 2048;

//     if (!src_hw) {
//         src_hw = dev->dev_addr;
//     }
//     if (!dest_hw) {
//         unsigned char boardcast = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//         dest_hw = boardcast;
//     }

//     if (dev_hard_header(skb, dev, ptype, dest_hw, src_hw, 2048) < 0)
//     {
//         return 0;
//     }
//     skb->len += 2048;

//     struct arphdr **arp = (buffer + offset);
//     (*arp)->ar_hrd = htons(ARPHRD_ETHER);
//     (*arp)->ar_pro = htons(ETH_P_IP);
//     (*arp)->ar_hln = 6;
//     (*arp)->ar_pln = 4;
//     (*arp)->ar_op = htons(type);

//     memcpy((*arp)->ar_sha, src_hw, sizeof((*arp)->ar_sha));
//     memcpy((*arp)->ar_sip, &src_ip, 4);

//     if (target_hw) {
//         memcpy((*arp)->ar_tha, target_hw, sizeof((*arp)->ar_tha));
//     } else {
//         memset((*arp)->ar_tha, 0, sizeof((*arp)->ar_tha));
//     }
//     memcpy((*arp)->ar_tha, &dest_ip, 4);

//     offset += sizeof(struct arphdr);

//     return buffer;
// }