#ifndef _ARCH_I386_H
#define _ARCH_I386_H

// #define PAGE_DIR_BASE 0x1000 //页目录存放地址
#define PAGE_TABLE_BASE 0x200000 //页表存放起始地址
#define PAGE_DIR_BASE PAGE_TABLE_BASE-0x1000 //页目录存放地址

/*----------------------------------------------------------------------------
; 分页机制使用的常量说明
;----------------------------------------------------------------------------
*/
#define PG_P		1	// 页存在属性位
#define PG_RWR		0	// R/W 属性位值, 读/执行
#define PG_RWW		2	// R/W 属性位值, 读/写/执行
#define PG_USS		0	// U/S 属性位值, 系统级
#define PG_USU		4	// U/S 属性位值, 用户级

struct PageDir{
    unsigned int entry[1024];
};

struct PageTable{
    unsigned int entry[1024];
};

#define get_pt_entry_p_addr(entry) ((struct PageTable*)((int)(entry) & 0xfffff000))
#define get_pt_entry_v_addr(entry) ((struct PageTable*)(__va(get_pt_entry_p_addr(entry))))

#define PDINDEX(virtualaddr) ((unsigned long)virtualaddr >> 22) //获取虚拟地址的目录页下标
#define PTINDEX(virtualaddr) ((unsigned long)virtualaddr >> PAGE_SHIFT & 0x03FF) //获取虚拟地址的页下标

#define invalidate() \
__asm__ __volatile__("movl %%cr3,%%eax\n\tmovl %%eax,%%cr3": : :"ax")

#define load_cr3(pgdir) \
    asm volatile("movl %0,%%cr3": :"r" (pgdir))

#endif