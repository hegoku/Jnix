#ifndef _NET_SKBUFFER_H
#define _NET_SKBUFFER_H

struct sk_buffer {
    struct net_device *dev;
    unsigned char *data;
    unsigned int len;
    struct sk_buffer *prev;
    struct sk_buffer *next;
};

int skb_free(struct sk_buffer *skb);
struct sk_buffer *skb_create(struct net_device *dev);
int skb_recv_push(struct sk_buffer *skb);
struct sk_buffer *skb_recv_shift();
int skb_handle_recv();
#endif