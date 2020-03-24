#ifndef _ARCH_I386_PCI_H_
#define _ARCH_I386_PCI_H_

#define PCI_COMD_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC

#define PCI_HEADER_TYPE(header_type) (header_type & 0x7F)
#define PCI_HEADER_TYPE_MF(header_type) (header_type & 0x80)

typedef struct s_pic_device_base_address_register {
    unsigned char map_type : 1;
    union u_pic_bar_layout{
        struct s_pic_bar_memory_psace {
            unsigned char type : 2;
            unsigned char prefetchable : 1;
            unsigned int address : 28;
        } __attribute__((packed)) mem_space;

        struct s_pic_bar_io_space {
            unsigned char reserved : 1;
            unsigned int address : 30;
        } __attribute__((packed)) io_space;
    } __attribute__((packed)) bar_layout;

} __attribute__((packed))PCI_DEVICE_BAR;

typedef struct s_pci_device_base_address {
    unsigned char map_type;
    unsigned char prefetchable;
    unsigned int address;
    unsigned int size;
    unsigned char address_size;
    struct s_pci_device_base_address *next;
} PCI_DEVICE_BA;

typedef struct s_pci_device_desc {
	unsigned short bus;
	unsigned short device;
	unsigned short func;

	unsigned short vendor_id;
	unsigned short device_id;

	unsigned char class_id;
	unsigned char subclass_id;
	unsigned char prog_if;
	unsigned char revision_id;

	unsigned char header_type;

    unsigned short subsystem_id;
    unsigned short subsystem_vendor_id;


    unsigned char interrupt_line;
    unsigned char interrupt_pin;
    unsigned char min_gant;
    unsigned char max_latency;

    PCI_DEVICE_BA *ba;

    struct s_pci_device_desc *next;
    // union u_pic_device_body {
    //     struct s_pic_device_header_type_0 {
    //         PCI_DEVICE_BAR bar0;
    //         PCI_DEVICE_BAR bar1;
    //         PCI_DEVICE_BAR bar2;
    //         PCI_DEVICE_BAR bar3;
    //         PCI_DEVICE_BAR bar4;
    //         PCI_DEVICE_BAR bar5;

    //         unsigned int CIS;

    //         unsigned short subsystem_id;
    //         unsigned short subsystem_vendor_id;

    //         unsigned int ROM_base_address;

    //         unsigned int reserved0 : 24;
    //         unsigned char capabilities_pointer;

    //         unsigned int reserved1;

    //         unsigned char max_latency;
    //         unsigned char min_grant;
    //         unsigned short interrupt;
    //     } header_type_0;
    // } body;
} PCI_DEVICE_DESC;

unsigned int pci_read(unsigned short bus, unsigned short device, unsigned short func, unsigned int register_offset);
void pci_write(unsigned short bus, unsigned short device, unsigned short func, unsigned int register_offset, unsigned int value);
unsigned char pci_device_has_func(unsigned short bus, unsigned short device);
void pci_select_drivers();
PCI_DEVICE_DESC *pci_get_device_desc(unsigned short bus, unsigned short device, unsigned short func);
PCI_DEVICE_DESC *pci_find_device(unsigned short vendor_id, unsigned short device_id);

#endif