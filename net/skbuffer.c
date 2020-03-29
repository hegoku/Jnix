#include <net/netdevice.h>
#include <system/mm.h>
#include <net/skbuffer.h>
#include <stdio.h>
#include <system/spinlock.h>

struct sk_buffer *recv_head = 0;
struct sk_buffer *recv_tail = 0;

static raw_spinlock_t lock = {
    lock : 1
};

struct sk_buffer *skb_create(struct net_device *dev)
{
    struct sk_buffer *skb = kzmalloc(sizeof(struct sk_buffer));
    skb->dev = dev;
    return skb;
}

int skb_free(struct sk_buffer *skb)
{
    // if (skb->prev) {
    //     skb->prev->next = skb->next;
    // }
    // if (skb->next) {
    //     skb->next->prev = skb->prev;
    // }
    // if (recv_head==skb) {
    //     recv_head = skb->next;
    // }
    // if (recv_tail==skb) {
    //     recv_tail = skb->prev;
    // }
    if (skb->data)
    {
        kfree(skb->data, skb->len);
    }
    kfree(skb, sizeof(struct sk_buffer));
    // printk("free\n");
    return 0;
}

int skb_recv_push(struct sk_buffer *skb)
{
    if (recv_tail) {
        recv_tail->next = skb;
        skb->prev = recv_tail;
        recv_tail = skb;
    } else {
        recv_head = skb;
        recv_tail = skb;
    }
    // printk("%x\n", recv_head);
    return 0;
}

struct sk_buffer *skb_recv_shift()
{
    struct sk_buffer *skb = recv_head;
    if (recv_head)
    {
        if (recv_head->next) {
            recv_head->next->prev = 0;
        }
        recv_head = recv_head->next;
        skb->prev = 0;
        skb->next = 0;
    }
    if (recv_head==0) {
        recv_tail = recv_head;
    }
    return skb;
}


int skb_handle_recv()
{
    struct sk_buffer *skb = skb_recv_shift();
    if (skb) {
    // while(skb!=0) {
        reveice_callback(skb->data, skb->len, skb->dev);
        skb_free(skb);
        // skb = skb_recv_shift();
    }
}