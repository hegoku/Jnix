#ifndef _SYSTEM_THREAD_H_
#define _SYSTEM_THREAD_H_

#include <fs/fs.h>

#define THREAD_TYPE_KERNEL 1
#define THREAD_TYPE_USER 0

#define TASK_RUNNING 0

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
    unsigned int retaddr; //return addr from kernel.asm::save()
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;
} STACK_FRAME;

typedef struct {
    unsigned int esp;
    unsigned int esp_addr;
    unsigned int eip;
} KERNEL_FRAME;

struct thread {
    STACK_FRAME regs;
    KERNEL_FRAME kernel_regs;
    unsigned int pid;
    unsigned int status;
    unsigned int flag;
    unsigned int parent_pid;
    unsigned char name[256];
    unsigned int alarm;


    struct file_descriptor **file_table;
    struct dir_entry *root;
    struct dir_entry *pwd;
    unsigned char exit_code;

    struct PageDir *page_dir;

    struct thread *prev;
    struct thread *next;
};

struct thread *thread_create(unsigned char *name, void *handle, int flag);
#endif