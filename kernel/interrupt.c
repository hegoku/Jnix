#include <system/interrupt.h>
#include <system/mm.h>

struct interrupt *interrupt_table = (struct interrupt *) 0;

// struct interrput* irq_table[256];

struct interrupt* __register_interrupt(unsigned char vector, void* handler, void *dev_id)
{
    struct interrupt *i;

    i = kzmalloc(sizeof(struct interrupt));
    i->vector = vector;
    i->handler = handler;
    i->dev_id = dev_id;

    // if (interrupt_table) {
    //     struct interrupt *next = interrupt_table->next;
    //     interrupt_table->next = i;
    //     i->next = next;
    // }
    // else
    // {
    //     interrupt_table=kzmalloc(sizeof(struct interrupt));
    //     interrupt_table->next = i;
    // }
    // irq_table[vector] = i;
    return i;
}
