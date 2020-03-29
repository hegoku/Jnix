#ifndef _SYSTEM_INTERRUPT_H
#define _SYSTEM_INTERRUPT_H

struct interrupt{
    unsigned char vector;
    void (*handler)(unsigned char irq, void *dev_id);
    void *dev_id;
    // struct interrupt *next;
};

struct interrupt *__register_interrupt(unsigned char irq, void *handler, void *dev_id);

#endif