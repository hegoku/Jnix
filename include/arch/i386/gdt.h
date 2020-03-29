#ifndef _ARCH_I386_GDT_H_
#define _ARCH_I386_GDT_H_

#include <arch/i386/desc.h>

#define GDT_SIZE 128
#define GDT_ADDR 0x1000 //新gdt地址，该地址原来在init中作为页目录地址

#define GDT_SEL_KERNEL_CODE (0x8|SA_RPL0) //因为loader的 GDT_SEL_CODE 选择子为 8
#define GDT_SEL_KERNEL_DATA (0x10|SA_RPL0)
#define GDT_SEL_VIDEO (0x18|SA_RPL3)
#define GDT_SEL_USER_CODE (0x20|SA_RPL3)
#define GDT_SEL_USER_DATA (0x28|SA_RPL3)
#define GDT_SEL_TSS (0x30|SA_RPL0)

#endif