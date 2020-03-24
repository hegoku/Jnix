#include <arch/i386/io.h>
#include <arch/i386/8295A.h>

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

// static __inline void eoi_8259A ()
// {
//     outb(0x20, INT_M_CTL);
//     __asm__("nop");
//     outb(0x20, INT_S_CTL);
// }