#include <arch/i386/io.h>
#include <system/timer.h>
#include <arch/i386/timer.h>
#include <arch/i386/idt.h>
#include <arch/i386/8295A.h>
#include <stdio.h>
#include <sys/types.h>

// static void __attribute__((interrupt)) timer_interrupt_handle(struct interrupt_frame *frame);
static void timer_interrupt_handle(int irq, void *dev_id);

void timer_init()
{
    outb(0x34, 0x43);
    outb((unsigned char)(TIMER_FREQ / HZ), 0x40);
    outb((unsigned char)((TIMER_FREQ / HZ)>>8), 0x40);

    register_irq(CLOCK_IRQ, timer_interrupt_handle, NULL);
    enable_8259A_irq(CLOCK_IRQ);
}

static void timer_interrupt_handle(int irq, void *dev_id)
{
    skb_handle_recv();
    // schedule();
    // printk("#");
}

// static void __attribute__((interrupt)) timer_interrupt_handle(struct interrupt_frame *frame)
// {
//     // __asm__ __volatile__ ("pushad\n
//     //     push ds\n
//     //     push es\n
//     //     push fs\n
//     //     push gs":::);
//     // printk("@");
//     disable_8259A_irq(CLOCK_IRQ);
    // skb_handle_recv();
//     // printk("!");
//     enable_8259A_irq(CLOCK_IRQ);
//     // __asm__ __volatile__ ("pop gs\n
//     //     pop fs\n
//     //     pop es\n
//     //     push ds\n
//     //     popad":::);
//     // printk("#");
// }