#ifndef _SYSTEM_THREAD_H_
#define _SYSTEM_THREAD_H_

typedef struct s_stackframe {
    unsigned int gs;
    unsigned int fs;
    unsigned int es;
    unsigned int ds;
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int kernel_esp; //popad will ignore it
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    // unsigned int retaddr; //return addr from kernel.asm::save()
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    // unsigned int esp;
    // unsigned int ss;
} STACK_FRAME;

typedef struct {
    unsigned int esp;
    unsigned int esp_addr;
    unsigned int eip;
} KERNEL_FRAME;

struct thread {
    unsigned int pid;
    unsigned int parent_pid;
    unsigned char name[256];
    unsigned int status;
    unsigned int alarm;

    struct PageDir *page_dir;

    STACK_FRAME regs;
    KERNEL_FRAME kernel_regs;

    struct thread *prev;
    struct thread *next;
};

extern struct thread *current_thread;

#endif