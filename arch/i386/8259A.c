#include <arch/i386/io.h>
#include <arch/i386/8295A.h>
#include <arch/i386/desc.h>
#include <arch/i386/idt.h>
#include <stdio.h>
#include <sys/types.h>

#define IRQ_NUMBER 16

struct interrupt{
    unsigned char irq;
    void (*handler)(int irq, void *dev_id);
    void *dev_id;
};

// irq_handler irq_table[IRQ_NUMBER] = {
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq,
//     spurious_irq
// };

struct interrupt irq_table[IRQ_NUMBER] = {
    {
        0,spurious_irq,NULL
    },
    {
        1,spurious_irq,NULL
    },
    {
        2,spurious_irq,NULL
    },
    {
        3,spurious_irq,NULL
    },
    {
        4,spurious_irq,NULL
    },
    {
        5,spurious_irq,NULL
    },
    {
        6,spurious_irq,NULL
    },
    {
        7,spurious_irq,NULL
    },
    {
        8,spurious_irq,NULL
    },
    {
        9,spurious_irq,NULL
    },
    {
        10,spurious_irq,NULL
    },
    {
        11,spurious_irq,NULL
    },
    {
        12,spurious_irq,NULL
    },
    {
        13,spurious_irq,NULL
    },
    {
        14,spurious_irq,NULL
    },
    {
        15,spurious_irq,NULL
    },
};

void init_8259A()
{
    outb(0x11, INT_M_CTL);			// Master 8259, ICW1.
	outb(0x11, INT_S_CTL);			// Slave  8259, ICW1.
	outb(INT_VECTOR_IRQ0, INT_M_CTLMASK);	// Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20.
	outb(INT_VECTOR_IRQ8, INT_S_CTLMASK);	// Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28
	outb(0x4, INT_M_CTLMASK);			// Master 8259, ICW3. IR2 对应 '从8259'.
	outb(0x2, INT_S_CTLMASK);			// Slave  8259, ICW3. 对应 '主8259' 的 IR2.
	outb(0x3, INT_M_CTLMASK);			// Master 8259, ICW4.
	outb(0x3, INT_S_CTLMASK);			// Slave  8259, ICW4.

	outb(0xFF, INT_M_CTLMASK);	// Master 8259, OCW1.
	outb(0xFF, INT_S_CTLMASK);	// Slave  8259, OCW1.

    enable_8259A_irq(CASCADE_IRQ); //开启从片

    init_idt_desc(INT_VECTOR_IRQ0+0, DA_386IGate, hwint00, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+1, DA_386IGate, hwint01, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+2, DA_386IGate, hwint02, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+3, DA_386IGate, hwint03, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+4, DA_386IGate, hwint04, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+5, DA_386IGate, hwint05, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+6, DA_386IGate, hwint06, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+7, DA_386IGate, hwint07, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+8, DA_386IGate, hwint08, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+9, DA_386IGate, hwint09, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+10, DA_386IGate, hwint10, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+11, DA_386IGate, hwint11, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+12, DA_386IGate, hwint12, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+13, DA_386IGate, hwint13, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+14, DA_386IGate, hwint14, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_IRQ0+15, DA_386IGate, hwint15, PRIVILEGE_KRNL);
}

void enable_8259A_irq(int irq)
{
    if (irq <= 8)
        outb(inb(INT_M_CTLMASK) & ~(1 << irq), INT_M_CTLMASK);
    else
        outb(inb(INT_S_CTLMASK) & ~(1 << (irq-8)), INT_S_CTLMASK);
}

void disable_8259A_irq(int irq)
{
	if(irq <= 8)
		outb(inb(INT_M_CTLMASK) | (1 << irq), INT_M_CTLMASK);
	else
		outb(inb(INT_S_CTLMASK) | (1 << (irq-8)), INT_S_CTLMASK);
}


void register_irq(unsigned char irq, void *handler, void *dev_id)
{
    irq_table[irq].handler = handler;
    irq_table[irq].dev_id = dev_id;
}

void spurious_irq(int irq, void *dev_id)
{
    printk("spurious_ %d\n", irq);
}
// static __inline void eoi_8259A ()
// {
//     outb(0x20, INT_M_CTL);
//     __asm__("nop");
//     outb(0x20, INT_S_CTL);
// }