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
    if ((current_thread->flag & THREAD_TYPE_KERNEL)==THREAD_TYPE_KERNEL) {
        current_thread->page_dir->entry[index1] = create_table(PG_P | PG_RWW | PG_USS);
        get_pt_entry_v_addr(current_thread->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USS;
    } else {
        current_thread->page_dir->entry[index1] = create_table(PG_P | PG_RWW | PG_USU);
        get_pt_entry_v_addr(current_thread->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USU;
    }
    
    mem_map[new_page >> 12].count++;

    printk("do_no_page pid(%d): addr:%x new:%x eip:%x esp:%x\n", current_thread->pid, address, new_page, current_thread->regs.eip, current_thread->regs.esp);

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

void copy_page(struct PageDir *pd, struct PageDir **res)
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 768; i++)
    {
        if (pd->entry[i]!= 0) {
            (*res)->entry[i] = create_table(PG_P | PG_RWW | PG_USU);
            // printk("%x %d ", (*res)->entry[i], i);while(1){}
            for (j = 0; j < 1024; j++)
            {
                if (get_pt_entry_v_addr(pd->entry[i])->entry[j]!= 0) {
                    get_pt_entry_v_addr(pd->entry[i])->entry[j] &= ~PG_RWW;
                    get_pt_entry_v_addr((*res)->entry[i])->entry[j] = get_pt_entry_v_addr(pd->entry[i])->entry[j];
                    mem_map[(get_pt_entry_v_addr(pd->entry[i])->entry[j] & 0xfffff000) >> 12].count++;
                    mem_map[(get_pt_entry_v_addr(pd->entry[i])->entry[j] & 0xfffff000) >> 12].flags |= MP_COW;
                }
                else
                {
                    get_pt_entry_v_addr((*res)->entry[i])->entry[j] = 0;
                }
            }
        }
        else
        {
            (*res)->entry[i] = 0;
        }
    }
    for (i = 768; i < 1024; i++)
    {
        (*res)->entry[i]= ((struct PageDir *)__va(PAGE_DIR_BASE))->entry[i];
    }
    invalidate();
}