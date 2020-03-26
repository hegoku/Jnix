#include <arch/i386/bios.h>
#include <arch/i386/page.h>
#include <system/page.h>
#include <arch/i386/desc.h>
#include <arch/i386/idt.h>
#include <arch/i386/8295A.h>
#include <arch/i386/pci.h>
#include <arch/i386/vga.h>
#include <arch/i386/bios.h>
#include <system/tty.h>
#include <stdio.h>
#include <arch/i386/am79c793.h>
#include <net/ether.h>
#include <net/arp.h>
#include <string.h>
#include <system/mm.h>
#include <net/netdevice.h>
#include <system/init.h>
#include <net/net.h>
#include <net/icmp.h>

void sendNet();

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
            a += 4096;
        }
        pg->entry[j+768] = __pa(((unsigned int)pt)) | PG_P | PG_RWW | PG_USS;
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
    struct s_gdtr gdt_ptr;
    __asm__ __volatile__("sgdt %0": "=m"(gdt_ptr)::);
    gdt_ptr.base = (void*)__va(gdt_ptr.base);
    __asm__ __volatile__("lgdt %0" ::"m"(gdt_ptr):);
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
    init_ldt();
    init_8259A();
    pci_select_drivers();
    pcibios_irq_init();

    do_initcalls();

    sendNet();
    while (1)
    {
    }
}

void sendNet()
{
    am79c793.ip=0x0a00020f;
    // icmp_echo(&am79c793, 1, 2, 0xc00a003A);
    icmp_echo(&am79c793, 1, 2, 0x0a000202);
    // arp_send(ARPOP_REQUEST, ETH_P_ARP, htonl(0x0a00020C), &am79c793, htonl(0x0a00020f), 0, 0, 0);
    while(1){}
    struct ethhdr aa;
    aa.h_dest[0] = 0xff;
    aa.h_dest[1] = 0xff;
    aa.h_dest[2] = 0xff;
    aa.h_dest[3] = 0xff;
    aa.h_dest[4] = 0xff;
    aa.h_dest[5] = 0xff;
    memcpy(aa.h_source, TO_AMDATA(am79c793.custom_data)->init_block.mac, sizeof(aa.h_source));
    aa.h_proto = htons(ETH_P_ARP);

    struct arphdr bb;
    bb.ar_hrd = htons(ARPHRD_ETHER);
    bb.ar_pro = htons(ETH_P_IP);
    bb.ar_hln = 6;
    bb.ar_pln = 4;
    bb.ar_op = htons(ARPOP_REQUEST);
    memcpy(bb.ar_sha, aa.h_source, sizeof(bb.ar_sha));
    bb.ar_sip[0] = 0x0a;
    bb.ar_sip[1] = 0x0;
    bb.ar_sip[2] = 0x2;
    bb.ar_sip[3] = 0xf;
    memset(bb.ar_tha, 0, sizeof(bb.ar_tha));
    bb.ar_tip[0] = 0x0a;
    bb.ar_tip[1] = 0x00;
    bb.ar_tip[2] = 0x2;
    bb.ar_tip[3] = 0x02;
    // bb.ar_tip[0] = 0xc0;
    // bb.ar_tip[1] = 0xa8;
    // bb.ar_tip[2] = 0x1;
    // bb.ar_tip[3] = 0x50;

    unsigned char *temp = kzmalloc(sizeof(bb)+sizeof(aa));
    memcpy(temp, &aa, sizeof(aa));
    memcpy(temp + sizeof(aa), &bb, sizeof(bb));

    // for (int i = 0; i < 5;i++) {

        am79c793.send(&am79c793, temp, sizeof(aa)+sizeof(bb));
    // }
}