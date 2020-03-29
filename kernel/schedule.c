#include <system/thread.h>
#include <system/page.h>
#include <arch/i386/io.h>
#include <arch/i386/page.h>

extern int is_in_ring0;
extern struct thread *current_thread;

void schedule()
{
    // printk("@");
    if (is_in_ring0 != 0)
    {
        return;
    }
    // printk("!");
    
    // disp_int((int)current_thread);
    struct thread *prev = current_thread;
    if (current_thread!=0) {
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
        // printk("1:%x %x %d %d\n",current_thread->kernel_regs.esp, prev->kernel_regs.esp, current_thread->pid, prev->pid);
        load_cr3(__pa(current_thread->page_dir));
        // switch_to(prev, current_thread, prev);
        // printk("2:%x %x %d %d\n",current_thread->kernel_regs.esp, prev->kernel_regs.esp, current_thread->pid, prev->pid);
    }
}