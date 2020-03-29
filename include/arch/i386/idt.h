
#ifndef _ARCH_I386_H_
#define _ARCH_I386_H_

#define IDT_SIZE 256

/* 中断向量 */
#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10
#define	INT_VECTOR_AC_ERR		0x11
#define	INT_VECTOR_MC_ERR		0x12
#define	INT_VECTOR_XF_ERR		0x13

#define	INT_VECTOR_SYS_CALL		0x80


#define load_ldt(ldtptr) \
    asm volatile("lidt %0"::"m" (*ldtptr))
    // asm volatile("lidt %0": :"r" (ldtptr))

typedef unsigned int uword_t __attribute__ ((mode (__word__)));
struct interrupt_frame
{
    uword_t eip;
    uword_t cs;
    uword_t eflags;
    uword_t esp;
    uword_t ess;
};

/* 中断处理函数 */
void divide_error();
void single_step_exception();
void nmi();
void breakpoint_exception();
void overflow();
void bounds_check();
void inval_opcode();
void copr_not_available();
void double_fault();
void copr_seg_overrun();
void inval_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void copr_error();

extern void sys_call();

void init_idt();
void register_interrupt(unsigned char vector, void *handler, void *dev_id);
void init_idt_desc(unsigned char vector, unsigned char desc_type, void *handler, unsigned char privilege);

#endif