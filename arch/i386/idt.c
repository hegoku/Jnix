#include <arch/i386/desc.h>
#include <arch/i386/idt.h>
#include <system/interrupt.h>
#include <system/page.h>
#include <stdio.h>



void __attribute__ ((interrupt)) devide_error(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) single_step_exception(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) nmi(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) breakpoint_exception(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) overflow(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) bounds_check(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) inval_opcode(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) copr_not_available(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) double_fault(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) copr_seg_overrun(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) inval_tss(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) segment_not_present(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) stack_exception(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) general_protection(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) page_fault(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) copr_error(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) ac_error(struct interrupt_frame* frame, uword_t error_code);
void __attribute__ ((interrupt)) mc_error(struct interrupt_frame* frame);
void __attribute__ ((interrupt)) xf_error(struct interrupt_frame* frame);

void __attribute__ ((interrupt)) sys_call(struct interrupt_frame* frame);
void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags);

// GATE *idt;
typedef struct s_idt_ptr {
    unsigned short limit;
    GATE *base;
} __attribute__((packed))IDT_PTR;
IDT_PTR idt_ptr;

/*----------------------------------------------------------------------*
 初始化 386 中断门
 *======================================================================*/
void init_idt_desc(unsigned char vector, unsigned char desc_type, void* handler, unsigned char privilege)
{
	GATE *	p_gate	= &idt_ptr.base[vector];
    unsigned int base = (unsigned int)handler;
    p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= 0x8;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (privilege << 5);
	p_gate->offset_high	= (base >> 16) & 0xFFFF;
}

void register_interrupt(unsigned char vector, void* handler, void *dev_id)
{
    __register_interrupt(vector, handler, dev_id);
    init_idt_desc(vector, DA_386IGate, handler, PRIVILEGE_KRNL);
}

void init_idt()
{
    idt_ptr.base = (GATE *)__va(get_free_page());
    get_free_page();
    get_free_page();
    get_free_page();
    idt_ptr.limit = 256 * sizeof(struct s_gate) -1;
    load_ldt(&idt_ptr);

    init_idt_desc(INT_VECTOR_DIVIDE, DA_386IGate, devide_error, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_DEBUG, DA_386IGate, single_step_exception, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_NMI, DA_386IGate, nmi, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception, PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_OVERFLOW,	DA_386IGate, overflow, PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_BOUNDS, DA_386IGate, bounds_check, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_INVAL_OP,	DA_386IGate, inval_opcode, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_NOT, DA_386IGate, copr_not_available, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate, double_fault, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGate, inval_tss, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGate, stack_exception, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_PROTECTION, DA_386IGate, general_protection, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_ERR, DA_386IGate, copr_error, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_AC_ERR, DA_386IGate, ac_error, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_MC_ERR, DA_386IGate, mc_error, PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_XF_ERR, DA_386IGate, xf_error, PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_SYS_CALL, DA_386IGate, sys_call, PRIVILEGE_USER);

    // register_interrupt(0, devide_error);
}

 
__attribute__((interrupt)) void devide_error (struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_DIVIDE, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) single_step_exception(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_DEBUG, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) nmi(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_NMI, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) breakpoint_exception(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_BREAKPOINT, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) overflow(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_OVERFLOW, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) bounds_check(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_BOUNDS, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) inval_opcode(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_INVAL_OP, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) copr_not_available(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_COPROC_NOT, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) double_fault(struct interrupt_frame* frame, uword_t error_code){
    exception_handler(INT_VECTOR_DOUBLE_FAULT, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) copr_seg_overrun(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_COPROC_SEG, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) inval_tss(struct interrupt_frame* frame, uword_t error_code){
    exception_handler(INT_VECTOR_INVAL_TSS, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) segment_not_present(struct interrupt_frame* frame, uword_t error_code){
    exception_handler(INT_VECTOR_SEG_NOT, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) stack_exception(struct interrupt_frame* frame, uword_t error_code){
    exception_handler(INT_VECTOR_STACK_FAULT, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) general_protection(struct interrupt_frame* frame, uword_t error_code){
    exception_handler(INT_VECTOR_PROTECTION, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) page_fault(struct interrupt_frame* frame, uword_t error_code){
    unsigned int page = 0;
    __asm__ __volatile__("movl %%cr2,%0"
                         : "=r" (page)
                         :
                         : "ax");
    printk("error_page: 0x%x  ", page);
    exception_handler(INT_VECTOR_PAGE_FAULT, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) copr_error(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_COPROC_ERR, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) ac_error(struct interrupt_frame* frame, uword_t error_code){
    exception_handler(INT_VECTOR_AC_ERR, error_code, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) mc_error(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_MC_ERR, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}
void __attribute__ ((interrupt)) xf_error(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_XF_ERR, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}

void __attribute__ ((interrupt)) sys_call(struct interrupt_frame* frame){
    exception_handler(INT_VECTOR_SYS_CALL, 0xFFFFFFFF, frame->eip, frame->cs, frame->eflags);
}

void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	int i;
	int text_color = 0x74; /* 灰底红字 */

	char * err_msg[] = {"#DE Divide Error",
			    "#DB RESERVED",
			    "--  NMI Interrupt",
			    "#BP Breakpoint",
			    "#OF Overflow",
			    "#BR BOUND Range Exceeded",
			    "#UD Invalid Opcode (Undefined Opcode)",
			    "#NM Device Not Available (No Math Coprocessor)",
			    "#DF Double Fault",
			    "    Coprocessor Segment Overrun (reserved)",
			    "#TS Invalid TSS",
			    "#NP Segment Not Present",
			    "#SS Stack-Segment Fault",
			    "#GP General Protection",
			    "#PF Page Fault",
			    "--  (Intel reserved. Do not use.)",
			    "#MF x87 FPU Floating-Point Error (Math Fault)",
			    "#AC Alignment Check",
			    "#MC Machine Check",
			    "#XF SIMD Floating-Point Exception"
	};

    printk("Exception! --> %s\n", err_msg[vec_no]);
    printk("EFLAGS:0x%x CS:0x%x EIP:0x%x\n", eflags, cs, eip);
    // DispColorStr("Exception! --> ", text_color);
    // DispColorStr(err_msg[vec_no], text_color);
	// DispColorStr("\n\n", text_color);
	// DispColorStr("EFLAGS:", text_color);
	// disp_int(eflags);
	// DispColorStr("CS:", text_color);
	// disp_int(cs);
	// DispColorStr("EIP:", text_color);
	// disp_int(eip);
	// DispColorStr("PID:", text_color);
	// disp_int(current_process->pid);

	if(err_code != 0xFFFFFFFF){
        printk("Error code:%x\n", err_code);
        // 	DispColorStr("Error code:", text_color);
        // 	disp_int(err_code);
    }
	while(1){}
}