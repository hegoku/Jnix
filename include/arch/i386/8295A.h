#ifndef _ARCH_I386_8295A_H_
#define _ARCH_I386_8295A_H_

/* 8259A interrupt controller ports. */
#define	INT_M_CTL	0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

#define INT_VECTOR_IRQ0 0x20
#define INT_VECTOR_IRQ8 0x28

#define	CLOCK_IRQ	0
#define	KEYBOARD_IRQ	1
#define	CASCADE_IRQ	2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	5	/* xt winchester */
#define	FLOPPY_IRQ	6	/* floppy disk */
#define	PRINTER_IRQ	7
#define	AT_WINI_IRQ	14	/* at winchester */

void init_8259A();

void enable_8259A_irq(int irq);
void disable_8259A_irq(int irq);

static __inline void eoi_8259A ()
{
    __asm__ __volatile__ ("outb %b0,%w1": :"a" (INT_M_CTL), "Nd" (0x20));
    __asm__("nop");
    __asm__ __volatile__ ("outb %b0,%w1": :"a" (INT_S_CTL), "Nd" (0x20));
}

#endif