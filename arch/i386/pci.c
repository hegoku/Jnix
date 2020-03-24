#include <stdio.h>
#include <string.h>
#include <system/mm.h>
#include <arch/i386/pci.h>
#include <arch/i386/io.h>

const char* PCI_CLASS_CODE[]={
	"Unclassified", "Mass Storage Controller", "Network Controller", "Display Controller", "Multimedia Controller", "Memory Controller", "Bridge Device", "Simple Communication Controller", "Base System Peripheral", "Input Device Controller", "Docking Station", "Processor", "Serial Bus Controller", "Wireless Controller", "Intelligent Controller", "Satellite Communication Controller", "Encryption Controller", "Signal Processing Controller", "Processing Accelerator", "Non-Essential Instrumentation", "0x3F (Reserved)", "Co-Processor", "0xFE (Reserved)", "Unassigned Class (Vendor specific)"
};

PCI_DEVICE_DESC *pci_device=0;

unsigned int pci_read(unsigned short bus, unsigned short device, unsigned short func, unsigned int register_offset)
{
	unsigned int id= 0x1<<31
		| ((bus & 0xFF) << 16)
		| ((device & 0x1F) << 11)
		| ((func & 0x07) << 8)
		| (register_offset & 0xFC);
	outl(id, PCI_COMD_PORT);
	unsigned int res=inl(PCI_DATA_PORT);
	return res >> (8* (register_offset % 4));
}

void pci_write(unsigned short bus, unsigned short device, unsigned short func, unsigned int register_offset, unsigned int value)
{
	unsigned int id= 0x1<<31
		| ((bus & 0xFF) << 16)
		| ((device & 0x1F) << 11)
		| ((func & 0x07) << 8)
		| (register_offset & 0xFC);
	outl(id, PCI_COMD_PORT);
	outl(value, PCI_DATA_PORT);
}

unsigned char pci_device_has_func(unsigned short bus, unsigned short device)
{
	return PCI_HEADER_TYPE_MF(pci_read(bus, device, 0, 0x0E));
}

void pci_select_drivers()
{
	for (int bus=0; bus<256;bus++){
		for (int device=0; device<32; device++) {
			unsigned char numFunctions=pci_device_has_func(bus, device)?8:1;
			for (int func=0; func<numFunctions; func++) {
				PCI_DEVICE_DESC *desc=pci_get_device_desc(bus, device, func);
                if (desc==0) continue;
                if (pci_device==0) {
                    pci_device = desc;
                } else {
                    desc->next = pci_device->next;
                    pci_device->next = desc;
                }
				// printk("PCI BUS:%x DEVICE:%x FUNC:%x VENDOR:%x DEVICE_ID:%x H:%x %s\n", bus, device, func, desc->vendor_id, desc->device_id, desc->header_type, PCI_CLASS_CODE[desc->class_id]);
                // PCI_DEVICE_BA *next = desc->ba;
                // while (next->next!=0)
                // {
                //     if (next->next->map_type==0) {
                //         printk("  Memory at %x (", next->next->address);
                //         if (next->next->address_size==0) {
                //             printk("32-bit, ");
                //         }
                //         else
                //         {
                //             printk("64-bit, ");
                //         }
                //         if (next->next->prefetchable==0) {
                //             printk("non-prefetchable)");
                //         } else {
                //             printk("prefetchable)");
                //         }
                //         printk(" [size=%d]\n", next->next->size);
                //     }
                //     else if (next->next->map_type == 1)
                //     {
                //         printk("  I/O port at %x [size=%d]\n", next->next->address, next->next->size);
                //     }
                //     next = next->next;
                // }

                // printk("%x %x %x\n", desc.interrupt, desc.body.header_type_0.interrupt, desc.body.header_type_0.bar0.map_type);
            }
        }
	}
}

PCI_DEVICE_DESC *pci_get_device_desc(unsigned short bus, unsigned short device, unsigned short func)
{
	PCI_DEVICE_DESC *res=kzmalloc(sizeof(PCI_DEVICE_DESC));
	res->bus=bus;
	res->device=device;	
	res->func=func;

	res->vendor_id=(unsigned short)pci_read(bus, device, func, 0x00);
	res->device_id=(unsigned short)pci_read(bus, device, func, 0x02);

    if (res->vendor_id==0x0000 || res->vendor_id==0xFFFF) {
        kfree(res, sizeof(PCI_DEVICE_DESC));
        return 0;
    }

    res->class_id=(unsigned char)pci_read(bus, device, func, 0x0b);
	res->subclass_id=(unsigned char)pci_read(bus, device, func, 0x0a);
	res->prog_if=(unsigned char)pci_read(bus, device, func, 0x09);

	res->revision_id=(unsigned char)pci_read(bus, device, func, 0x08);
    unsigned int interrupt=pci_read(bus, device, func, 0x3C);
    res->interrupt_line = (unsigned char)interrupt & 0xFF;
    res->interrupt_pin = (unsigned char)(interrupt>>8) & 0xFF;

    res->header_type=(unsigned char)pci_read(bus, device, func, 0x0E);

    res->ba=kzmalloc(sizeof(PCI_DEVICE_BA));

    // unsigned int conf = pci_read(res->bus, res->device, res->func, 0x4);
    // conf &= 0xffff0000; // preserve status register, clear config register
    // conf |= 0x5;        // set bits 0 and 2
    // pci_write(res->bus, res->device, res->func, 0x4, conf);

    if (PCI_HEADER_TYPE(res->header_type)==0) {
        for (int bar = 0; bar < 6; bar++) {
            // printk("%x %x %x %x %x %x\n", pci_read(bus, device, func, 0x10), pci_read(bus, device, func, 0x14), pci_read(bus, device, func, 0x18), pci_read(bus, device, func, 0x1C), pci_read(bus, device, func, 0x20), pci_read(bus, device, func, 0x24));
            // res.body.header_type_0.bar0= memset(res.body.header_type_0.bar0, pci_read(bus, device, func, 0x10), sizeof PCI_DEVICE_BAR);
            int offset = 0x10 + 4 * bar;
            // PCI_DEVICE_BAR origin;
            // memset(&res.body.header_type_0.bar0, pci_read(bus, device, func, 0x10), sizeof(PCI_DEVICE_BAR));
            // memset(&origin, pci_read(bus, device, func, offset), sizeof(PCI_DEVICE_BAR));
            unsigned int origin = pci_read(bus, device, func, offset);

            PCI_DEVICE_BA *ba = kzmalloc(sizeof(PCI_DEVICE_BA));
            if ((origin & 0x1) == 0)
            { //mmio
                ba->map_type = 0;
                ba->prefetchable =  (origin & 0x8) ? 1: 0;
                switch ((origin>>1) & 0x3)
                {
                case 0x0: //32bit memory
                    pci_write(bus, device, func, offset, 0xFFFFFFFF);
                    ba->size = pci_read(bus, device, func, offset) & 0xFFFFFFF0;
                    ba->size = ~ba->size + 1;
                    // printk("bar%d:%x ", bar, origin);
                    // res.address_space0 = bar_value;
                    if (ba->size == 0)
                    {
                        kfree(ba, sizeof(PCI_DEVICE_BA));
                        continue;
                    }
                    ba->address_size = 0;
                    break;
                case 0x1: //20bit memory
                    break;
                case 0x2: //64bit
                    ba->address_size = 1;
                    break;
                }
                ba->address = origin & 0xFFFFFFF0;
            }
            else
            { //io
                ba->map_type = 1;
                ba->prefetchable = 0;
                pci_write(bus, device, func, offset, 0xFFFFFFFF);
                ba->size = pci_read(bus, device, func, offset) & 0xFFFFFFFC;
                ba->size = ~ba->size +1;
                if (ba->size==0) {
                    kfree(ba, sizeof(PCI_DEVICE_BA));
                    continue;
                }
                ba->address = origin & 0xFFFFFFFC;
                // printk("bar%d:%x ", bar, ba->size);
            }
            pci_write(bus, device, func, offset, origin);

            PCI_DEVICE_BA *next = res->ba;
            while (next->next != 0)
            {
                next = next->next;
            }
            next->next = ba;
        }
        
    }

    return res;
}

PCI_DEVICE_DESC* pci_find_device(unsigned short vendor_id, unsigned short device_id)
{
    PCI_DEVICE_DESC *next = pci_device;
    while (next) {
        if (next->vendor_id==vendor_id && next->device_id==device_id) {
            return next;
        }
        next = next->next;
    }
    return 0;
}