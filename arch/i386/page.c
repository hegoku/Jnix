#include <arch/i386/page.h>
#include <system/page.h>
#include <system/thread.h>
#include <string.h>

extern struct thread *current_thread;
extern struct Page *mem_map;

void do_wp_page(unsigned int error_code, unsigned int address)
{
    int index1 = address >> 22;
    int index2 = address >> 12 & 0x03FF;
    unsigned int old_page, new_page;
    old_page = 0xfffff000 & get_pt_entry_v_addr(current_thread->page_dir->entry[index1])->entry[index2];
    if (mem_map[old_page >> 12].count==1) {
        get_pt_entry_v_addr(current_thread->page_dir->entry[index1])->entry[index2] = old_page | PG_P | PG_RWW | PG_USU;
        mem_map[old_page >> 12].flags |= MP_USE;
        new_page = old_page;
    }
    else
    {
        new_page = get_free_page();
        get_pt_entry_v_addr(current_thread->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USU;
        memcpy((void *)__va(new_page), (void *)__va(old_page), 1024 * 4);
        mem_map[new_page >> 12].count++;
        mem_map[old_page >> 12].count--;
    }

    // printk("do_wap_page pid(%d): addr:%x old:%x new:%x %x eip:%x esp:%x\n", current_thread->pid, address, old_page, new_page, old_page >> 12, current_thread->regs.eip, current_thread->regs.esp);

    invalidate();

    if (current_thread->pid==2) {
        // while(1){}
    }
}

void do_no_page(unsigned int error_code, unsigned int address)
{
    int index1 = address >> 22;
    int index2 = address >> 12 & 0x03FF;
    unsigned int new_page;

    new_page = get_free_page();
    current_thread->page_dir->entry[index1] = create_table(PG_P | PG_RWW | PG_USU);
    get_pt_entry_v_addr(current_thread->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USU;
    mem_map[new_page >> 12].count++;

    // printk("do_no_page pid(%d): addr:%x new:%x eip:%x esp:%x\n", current_thread->pid, address, new_page, current_thread->regs.eip, current_thread->regs.esp);

    invalidate();

    // if (current_thread->pid==2) {
        // while(1){}
    // }
}

inline struct PageDir *create_dir()
{
    return (struct PageDir*)(__va(get_free_page()));
}

inline unsigned int create_table(unsigned int attr)
{
    return ((get_free_page())|attr);
}