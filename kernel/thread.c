#include <system/mm.h>
#include <system/page.h>
#include <system/thread.h>
#include <string.h>
#include <stdio.h>
#include <arch/i386/desc.h>
#include <arch/i386/gdt.h>
#include <sys/types.h>
#include <system/init.h>

#define INDEX_LDT_CS 0
#define INDEX_LDT_DS 1
#define PROC_STACK_TOP 0x8000000

extern void restart();
struct thread *thread_table=0;
struct thread *current_thread=0;
unsigned int pid=0;

int thread_int()
{
    current_thread=thread_table;
}

struct thread* thread_create(unsigned char *name, void *handle, int flag)
{
    struct thread *th=kzmalloc(sizeof(struct thread));
    th->pid=pid;
    th->parent_pid=-1;
    pid++;

    sprintf(th->name, name);

    // th->page_dir=(struct PageDir *)__va(PAGE_DIR_BASE);
    th->page_dir=NULL;
    th->flag=flag;
    
    if ((flag&THREAD_TYPE_KERNEL)==THREAD_TYPE_KERNEL) {
        th->regs.cs = (GDT_SEL_KERNEL_CODE&SA_RPL_MASK) | SA_RPL0;
        th->regs.ds = (GDT_SEL_KERNEL_DATA&SA_RPL_MASK) | SA_RPL0;
        th->regs.es = (GDT_SEL_KERNEL_DATA&SA_RPL_MASK) | SA_RPL0;
        th->regs.fs = (GDT_SEL_KERNEL_DATA&SA_RPL_MASK) | SA_RPL0;
        th->regs.ss = (GDT_SEL_KERNEL_DATA&SA_RPL_MASK) | SA_RPL0;
        th->regs.gs = (GDT_SEL_VIDEO & SA_RPL_MASK) | SA_RPL0;
        th->regs.kernel_esp=__va(get_free_page()+1024*4);
        printk("%x %x \n", th->regs.gs, &(th->regs.gs));
    } else {
        th->regs.cs = (GDT_SEL_USER_CODE&SA_RPL_MASK) | SA_RPL3;
        th->regs.ds = (GDT_SEL_USER_DATA&SA_RPL_MASK) | SA_RPL3;
        th->regs.es = (GDT_SEL_USER_DATA&SA_RPL_MASK) | SA_RPL3;
        th->regs.fs = (GDT_SEL_USER_DATA&SA_RPL_MASK) | SA_RPL3;
        th->regs.ss = (GDT_SEL_USER_DATA&SA_RPL_MASK) | SA_RPL3;
        th->regs.gs = (GDT_SEL_VIDEO & SA_RPL_MASK) | SA_RPL3;
        th->regs.esp = PROC_STACK_TOP;
    }
    
    th->regs.eip = (unsigned int)handle;

    th->regs.eflags = 0x3202;

    th->status = 0;
    th->parent_pid = -1;

    th->file_table = kzmalloc(sizeof(void*) * PROC_FILES_MAX_COUNT);

    if (thread_table == 0)
    {
        thread_table=th;
        th->next=th;
        th->prev=th;
    }
    else
    {
        th->next=thread_table;
        th->prev=thread_table->prev;
        thread_table->prev->next=th;
        thread_table->prev=th;
    }
    current_thread=th;
    return th;
}

core_initcall(thread_int);


pid_t sys_fork()
{
    int pid = -1;
    int i;
    struct thread *p=thread_create("", NULL, current_thread->flag);

    *p=*current_thread;
    p->parent_pid=current_thread->pid;
	sprintf(p->name, "%s_%d", current_thread->name, p->pid);
	p->regs.eax = 0;
	p->status = TASK_RUNNING;
    p->page_dir = create_dir();

    copy_page(p->page_dir, &(current_thread->page_dir));

    //文件描述符
    for (i = 0; i < PROC_FILES_MAX_COUNT; i++)
    {
        if (p->file_table[i]) {
            p->file_table[i]->inode->used_count++;
            p->file_table[i]->used_count++;
        }
    }
    p->kernel_regs.eip = (unsigned int)restart;
    p->kernel_regs.esp_addr = __va(get_free_page());
    p->kernel_regs.esp = __pa(p->kernel_regs.esp_addr) + 1024 * 4;

    // printk("fork pid(%d): %x %x eip:%x esp:%x\n", current_process->pid, process_table[pid].kernel_regs.esp_addr, process_table[pid].kernel_regs.esp, current_process->regs.eip, current_process->regs.esp);
    
    return pid;
}
