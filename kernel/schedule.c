#include <system/thread.h>
#include <system/page.h>
#include <arch/i386/io.h>
#include <arch/i386/page.h>

extern int is_in_ring0;
extern struct thread *current_thread;

#define switch_to(prev, next, last) \
    { \
        __asm__ ("pushfl\n\t" \
        "pushl %%eax\n\t" \
        "pushl %%ecx\n\t" \
        "pushl %%edx\n\t" \
        "pushl %%ebx\n\t" \
        "pushl %%ebp\n\t" \
        "pushl %%esi\n\t" \
        "pushl %%edi\n\t" \
        "movl %%esp, %[prev_sp]\n\t" \
        "movl %[next_sp], %%esp\n\t" \
        "movl $1f, %[prev_ip]\n\t" \
        "jmp %[next_ip]\n\t" \
        "1:\t" \
        "popl %%edi\n\t" \
        "popl %%esi\n\t" \
        "popl %%ebp\n\t" \
        "popl %%ebx\n\t" \
        "popl %%edx\n\t" \
        "popl %%ecx\n\t" \
        "popl %%eax\n\t" \
        "popfl\n\t" \
        : [prev_sp] "=m" ((prev)->kernel_regs.esp), \
          [prev_ip] "=m" ((prev)->kernel_regs.eip) \
        : [next_sp] "m" ((next)->kernel_regs.esp), \
          [next_ip] "m" ((next)->kernel_regs.eip) \
        : "memory" \
        ); \
    }

void schedule()
{
    // printk("@");
    // if (is_in_ring0 != 0)
    // {
    //     return;
    // }
    // printk("!");
    
    // disp_int((int)current_thread);
    struct thread *prev = current_thread;
    if (current_thread==0) {
        return;
    }
    // for (int i = 0; i < PROC_NUMBER;i++) {
    //     if (process_table[i].is_free == 1)
    //     {
    //         if (process_table[i].alarm && process_table[i].alarm < ticks)
    //         {
    //             process_table[i].signal |= (1 << (SIGALRM - 1));
    //             process_table[i].alarm = 0;
    //         }
    //         if (process_table[i].signal && process_table[i].status == TASK_INTERRUPTIBLE)
    //         {
    //             process_table[i].status = TASK_RUNNING;
    //         }
    //     }
    // }
    // do
    // {
    //     current_thread=current_thread->next;
    // } while (current_thread->is_free != 1 || current_thread->status != TASK_RUNNING);
    current_thread=current_thread->next;
    if (prev!=current_thread) {
        printk("1:%x %x %d %d\n",current_thread->kernel_regs.esp, prev->kernel_regs.esp, current_thread->pid, prev->pid);
        load_cr3(__pa(current_thread->page_dir));
        switch_to(prev, current_thread, prev);
        printk("2:%x %x %d %d\n",current_thread->kernel_regs.esp, prev->kernel_regs.esp, current_thread->pid, prev->pid);
    }
}