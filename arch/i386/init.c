#include <arch/i386/bios.h>
#include <arch/i386/page.h>
#include <system/page.h>
#include <arch/i386/desc.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/8295A.h>
#include <arch/i386/pci.h>
#include <arch/i386/vga.h>
#include <arch/i386/bios.h>
#include <system/tty.h>
#include <stdio.h>
#include <string.h>
#include <system/mm.h>
#include <arch/i386/timer.h>

TSS tss={
    // .esp0 = TOP_OF_KERNEL_STACK,
	.ss0 = GDT_SEL_KERNEL_DATA
};

// unsigned short swap_uint16(unsigned short val) {
//     return (val << 8) | (val >> 8);
// }

// short swap_int16(short val) {
//     return (val << 8) | ((val >> 8) & 0xFF);
// }

// unsigned int swap_uint32(unsigned int val) {
//     val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
//     return (val << 16) | (val >> 16);
// }

// int swap_int32(int val) {
//     val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
//     return (val << 16) | ((val >> 16) & 0xFFFF);
// }

// GATE *idt;

static int load_memory_size()
{
    int mem_size = 0;
    unsigned short MCRNumber = *(unsigned short *)(ARDS_MEMBER_BASE);
    struct ARDStruct *MemChkBuf = (void *)(ARDS_DATA);
    for (int i = 0; i < MCRNumber; i++)
    {
        if (MemChkBuf->type == ARD_TYPE_FREE)
        {
            if (MemChkBuf->baseAddrLow+MemChkBuf->lengthLow>mem_size) {
                mem_size = MemChkBuf->baseAddrLow + MemChkBuf->lengthLow;
            }
        }
        // printk("%x %x %x %x %x\n", MemChkBuf->baseAddrLow, MemChkBuf->baseAddrHigh, MemChkBuf->lengthLow, MemChkBuf->lengthHigh, MemChkBuf->type);
        MemChkBuf += 1;
    }
    return mem_size;
    // printk("mmap:%x\n", mmap);
}

static void reset_paging(int mem_size)
{
    // unsigned int high_mem = 0;
    // //192M<=a<=896M（低端必须至少64M，所以128M+64M=192M），（如果打开CONFIG_HIGHMEM）则高端内存为[a-128M,a]
    // if (mem_size>=192*1024*1024 && mem_size<=HIGH_MEMORY) {
    //     high_mem = mem_size - 128 * 1024 * 1024;
    // }
    // //如果a>896M，则高端内存为[896M，a]
    // if (mem_size>HIGH_MEMORY) {
    //     high_mem = HIGH_MEMORY;
    // }

    unsigned int page_table_count; //页目录个数
    if (HIGH_MEMORY > mem_size) {
        page_table_count = (mem_size + 0x400000 - 1) / 0x400000;
    } else {
        page_table_count = (HIGH_MEMORY + 0x400000 - 1) / 0x400000;
    }

    unsigned int a = 0;
    int j = 0;
    int i = 0;

    //分配1G的内核空间，即将mem_size的物理内存映射到 虚拟内存的 3G-4G
    struct PageTable *pt = (struct PageTable *)__va(PAGE_TABLE_BASE);
    struct PageDir *pg = (struct PageDir *)__va(PAGE_DIR_BASE);
    for (j = 0; j < page_table_count; j++) //768是3G虚拟内存所在的页目录下表
    {
        for (i = 0; i < 1024; i++)
        {
            pt->entry[i] = a | PG_P | PG_RWW | PG_USS;
            // pt->entry[i] = a | PG_P | PG_RWW | PG_USU;
            a += 4096;
        }
        pg->entry[j+768] = __pa(((unsigned int)pt)) | PG_P | PG_RWW | PG_USS;
        // pg->entry[j+768] = __pa(((unsigned int)pt)) | PG_P | PG_RWW | PG_USU;
        pt++;
    }
    load_cr3(PAGE_DIR_BASE);

    // page_table_count = (mem_size + 0x400000 - 1) / 0x400000;
    mem_map=(struct Page*)__va(PAGE_TABLE_BASE+0x100000);
    struct Page *page_ptr = mem_map;
    unsigned int mem_map_size = sizeof(struct Page) * 1024 * page_table_count/(1024*4);
    mem_map_count = page_table_count * 1024;
    for (i = 0; i < mem_map_count; i++)
    {
        if (i<(1024+mem_map_size)) { //内存开始4MB+mem_map的大小标记为已使用
            page_ptr->count = 1;
            page_ptr->flags |= MP_USE;
        } else {
            page_ptr->count = 0;
            page_ptr->flags = 0;
        }
        page_ptr++;
    }

    //开启分页后，原来的gdtr的base要加上PAGE_OFFSET
    // struct s_gdtr gdt_ptr;
    // __asm__ __volatile__("sgdt %0": "=m"(gdt_ptr)::);
    // gdt_ptr.base = (void*)__va(gdt_ptr.base);
    // __asm__ __volatile__("lgdt %0" ::"m"(gdt_ptr):);
}

static void reset_gdt()
{
    struct s_gdtr gdt_ptr;
    gdt_ptr.base = (void *)__va(GDT_ADDR);
    gdt_ptr.limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;

    DESCRIPTOR *gdt = (DESCRIPTOR *)gdt_ptr.base;

    DESCRIPTOR gdt_0 = create_descriptor(0, 0, 0);
	insert_descriptor(gdt, 0, gdt_0, PRIVILEGE_KRNL);

	DESCRIPTOR kernel_cs = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL0);
    insert_descriptor(gdt, 1, kernel_cs, PRIVILEGE_KRNL);

    DESCRIPTOR kernel_ds = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL0);
	insert_descriptor(gdt, 2, kernel_ds, PRIVILEGE_KRNL);

	DESCRIPTOR video = create_descriptor(0xB8000, 0xBFFFF, DA_DRW | DA_32 | DA_DPL3);
	insert_descriptor(gdt, 3, video, PRIVILEGE_USER);

	DESCRIPTOR user_cs = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
	insert_descriptor(gdt, 4, user_cs, PRIVILEGE_USER);

	DESCRIPTOR user_ds = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
	insert_descriptor(gdt, 5, user_ds, PRIVILEGE_USER);

    DESCRIPTOR tss_desc = create_descriptor((unsigned int)&tss, sizeof(TSS) - 1, DA_386TSS);
    insert_descriptor(gdt, 6, tss_desc, PRIVILEGE_KRNL);

    __asm__ __volatile__("lgdt %0" ::"m"(gdt_ptr):);
    __asm__ __volatile__("movl %0, %%eax;ltr %%ax"
                         : :"g"(GDT_SEL_TSS):"%eax");
}

// void init_ldt()
// {
//     idt = get_free_page();
//     // 全部初始化成中断门(没有陷阱门)
//     init_idt_desc(__va(idt), INT_VECTOR_DIVIDE, DA_386IGate, divide_error, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_DEBUG, DA_386IGate, single_step_exception, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_NMI, DA_386IGate, nmi, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception, PRIVILEGE_USER);

// 	init_idt_desc(__va(idt), INT_VECTOR_OVERFLOW,	DA_386IGate, overflow, PRIVILEGE_USER);

// 	init_idt_desc(__va(idt), INT_VECTOR_BOUNDS, DA_386IGate, bounds_check, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_INVAL_OP,	DA_386IGate, inval_opcode, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_COPROC_NOT, DA_386IGate, copr_not_available, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_DOUBLE_FAULT,	DA_386IGate, double_fault, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_INVAL_TSS, DA_386IGate, inval_tss, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_STACK_FAULT, DA_386IGate, stack_exception, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_PROTECTION, DA_386IGate, general_protection, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_COPROC_ERR, DA_386IGate, copr_error, PRIVILEGE_KRNL);

// 	init_idt_desc(__va(idt), INT_VECTOR_SYS_CALL, DA_386IGate, sys_call, PRIVILEGE_USER);

//     load_ldt(idt);
// }

void init_arch()
{
    init_tty();
    int mem_size = load_memory_size();
    reset_paging(mem_size);
    reset_gdt();
    init_idt();
    init_8259A();
    pci_select_drivers();
    pcibios_irq_init();
    timer_init();
}