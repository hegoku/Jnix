#ifndef _ARCH_I386_BIOS_H_
#define _ARCH_I386_BIOS_H_

#define BIOS_PARAMS_BASE 0x7e00
#define ARDS_MEMBER_BASE BIOS_PARAMS_BASE
#define ARDS_DATA ARDS_MEMBER_BASE+2

#define PIRQ_SIGNATURE (('$' << 0) + ('P' << 8) + ('I' << 16) + ('R' << 24))
#define PIRQ_VERSION 0x0100

struct ARDStruct{
    unsigned int baseAddrLow;
    unsigned int baseAddrHigh;
    unsigned int lengthLow;
    unsigned int lengthHigh;
    unsigned int type;
}__attribute__ ((packed));

#define ARD_TYPE_FREE 1
#define ADR_TYPE_RESERVED 2

struct irq_info {
    //设备的总线号和设备功能号
    unsigned char bus, devfn;                     /* Bus, device and function */
    //pci设备的每个引脚对应的输入线和可能的IRQ号
    struct {
            unsigned char link;               /* IRQ line ID, chipset dependent, 0=not routed */
            unsigned short bitmap;                /* Available IRQs */
    } __attribute__((packed)) irq[4];
    unsigned char slot;                        /* Slot number, 0=onboard */
    unsigned char rfu;
} __attribute__((packed));

struct irq_routing_table {
        //恒定义为PIRQ_SIGNATURE
        unsigned int signature;                    /* PIRQ_SIGNATURE should be here */
        //恒为PIRQ_VERSION
        unsigned short version;                        /* PIRQ_VERSION */
        //表的大小
        unsigned short size;                     /* Table size in bytes */
        //PIR的总线号和设备功能号
        unsigned char rtr_bus, rtr_devfn;                   /* Where the interrupt router lies */
        //分配给PIR专用的IRQ
        unsigned short exclusive_irqs;            /* IRQs devoted exclusively to PCI usage */
        //PIR芯片的厂商和设备号
        unsigned short rtr_vendor, rtr_device;         /* Vendor and device ID of interrupt router */
        //没有用到
        unsigned int miniport_data;            /* Crap */
        //保留区
        unsigned char rfu[11];
        //检验和.表头与检验之和为0
        unsigned char checksum;                     /* Modulo 256 checksum must give zero */
        //中断路径表项,包含了每个设备对应的输入引脚和可用的IRQ号等信息
        struct irq_info slots[0];
}__attribute__((packed));

static struct irq_routing_table *pirq_find_routing_table();
static inline struct irq_routing_table *pirq_check_routing_table(unsigned char *addr);
static void pirq_peer_trick();
static void pcibios_fixup_irqs();
int pcibios_irq_init();

#endif