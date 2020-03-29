#ifndef _SYSTEM_PAGE_H_
#define _SYSTEM_PAGE_H

#define PAGE_OFFSET 0xC0000000 // 3GB   内核入口地址为0xC0100000-0xc0150000 (320K)   0xc0150000开始的1MB给kmalloc
#define PAGE_SIZE 1024*4
#define PAGE_SHIFT 12

#define HIGH_MEMORY 0x38000000 //896MB

#define PG_P_ADDR unsigned int //物理页的物理地址
#define PG_KV_ADDR unsigned int //物理页在内核空间的虚拟地址

#define __pa(v_addr) ((int)(v_addr)-PAGE_OFFSET)
#define __va(p_addr) ((int)(p_addr)+PAGE_OFFSET)
#define get_pt_entry_p_addr(entry) ((struct PageTable*)((int)(entry) & 0xfffff000))
#define get_pt_entry_v_addr(entry) ((struct PageTable*)(__va(get_pt_entry_p_addr(entry))))

#define MP_USE 1 //被使用
#define MP_COW 2

struct Page {
    unsigned int count;
    unsigned int flags;
};

extern unsigned int mem_map_count;
extern struct Page *mem_map;

int get_free_page();
void free_page(unsigned int pyhsics_addr);
#endif