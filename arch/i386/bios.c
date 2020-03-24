#include <system/page.h>
#include <stdio.h>
#include <arch/i386/bios.h>
#include <string.h>

struct irq_routing_table *pirq_table;

/*
 * Never use: 0, 1, 2 (timer, keyboard, and cascade)
 * Avoid using: 13, 14 and 15 (FP error and IDE).
 * Penalize: 3, 4, 6, 7, 12 (known ISA uses: serial, floppy, parallel and mouse)
 */
unsigned int pcibios_irq_mask = 0xfff8;
static int pirq_penalty[16] = {
	1000000, 1000000, 1000000, 1000, 1000, 0, 1000, 1000,
	0, 0, 0, 0, 1000, 100000, 100000, 100000
};

int pcibios_irq_init()
{
    pirq_table = pirq_find_routing_table();
     if (pirq_table) {
        pirq_peer_trick();
        // pirq_find_router(&pirq_router);
        if (pirq_table->exclusive_irqs) {
            int i;
            for (i=0; i<16; i++)
                if (!(pirq_table->exclusive_irqs & (1 << i)))
                        pirq_penalty[i] += 100;
        }
        /* If we're using the I/O APIC, avoid using the PCI IRQ routing table */
        // if (io_apic_assign_pci_irqs)
        //         pirq_table = 0;
    }

    // pcibios_enable_irq = pirq_enable_irq;

    // pcibios_fixup_irqs();
    return 0;
}

static struct irq_routing_table * pirq_find_routing_table()
{
    unsigned char *addr;
    struct irq_routing_table *rt;

    for(addr = (unsigned char *) __va(0xf0000); addr < (unsigned char *) __va(0x100000); addr += 16) {
        rt = pirq_check_routing_table(addr);
        if (rt) {
            printk("size:%d bus:0x%x, dev:0x%x exclusive_irqs:0x%x\n", rt->size, rt->rtr_bus, rt->rtr_devfn, rt->exclusive_irqs);
            return rt;
        }
    }
    return 0;
}

static inline struct irq_routing_table * pirq_check_routing_table(unsigned char *addr)
{
    struct irq_routing_table *rt;
    int i;
    unsigned char sum;

    rt = (struct irq_routing_table *) addr;
    if (rt->signature != PIRQ_SIGNATURE ||
        rt->version != PIRQ_VERSION ||
        //是否低四位对齐
        rt->size % 16 ||                                                
        rt->size < sizeof(struct irq_routing_table))
            return 0;
    sum = 0;
    //所有的值加起来必须为零.因为它后面多了一个checksum
    for (i=0; i < rt->size; i++)
            sum += addr[i];
    if (!sum) {
            printk("PCI: Interrupt Routing Table found at 0x%p\n", rt);
            return rt;
    }
    return 0;
}

static void pirq_peer_trick()
{
    struct irq_routing_table *rt = pirq_table;
    unsigned char busmap[256];
    int i;
    struct irq_info *e;

    memset(busmap, 0, sizeof(busmap));
    for(i=0; i < (rt->size - sizeof(struct irq_routing_table)) / sizeof(struct irq_info); i++) {
        e = &rt->slots[i];
#ifdef DEBUG
        {
                int j;
                printk("%02x:%02x slot=%02x", e->bus, e->devfn/8, e->slot);
                for(j=0; j<4; j++)
                            printk(" %d:%02x/%04x", j, e->irq[j].link, e->irq[j].bitmap);
                printk("\n");
        }
#endif
        busmap[e->bus] = 1;
    }
    // for(i = 1; i < 256; i++) {
    //     if (!busmap[i] || pci_find_bus(0, i))
    //             continue;
    //     if (pci_scan_bus_with_sysdata(i))
    //             printk("PCI: Discovered primary peer "
    //                     "bus %02x [IRQ]\n", i);
    // }
    // pcibios_last_bus = -1;
}

// static void pirq_find_router(struct irq_router *r)
// {
//     struct irq_routing_table *rt = pirq_table;
//     struct irq_router_handler *h;
 
// #ifdef CONFIG_PCI_BIOS
//     if (!rt->signature) {
//             printk(KERN_INFO "PCI: Using BIOS for IRQ routing\n");
//             r->set = pirq_bios_set;
//             r->name = "BIOS";
//             return;
//     }
// #endif

//     /* Default unless a driver reloads it */
//     r->name = "default";
//     r->get = 0;
//     r->set = 0;

//     printk("PCI: Attempting to find IRQ router for %04x:%04x\n",
//         rt->rtr_vendor, rt->rtr_device);

//     pirq_router_dev = pci_get_bus_and_slot(rt->rtr_bus, rt->rtr_devfn);
//     if (!pirq_router_dev) {
//         printk("PCI: Interrupt router not found at "
//                 "%02x:%02x\n", rt->rtr_bus, rt->rtr_devfn);
//         return;
//     }

//     for( h = pirq_routers; h->vendor; h++) {
//         /* First look for a router match */
//         if (rt->rtr_vendor == h->vendor && h->probe(r, pirq_router_dev, rt->rtr_device))
//                 break;
//         /* Fall back to a device match */
//         if (pirq_router_dev->vendor == h->vendor && h->probe(r, pirq_router_dev, pirq_router_dev->device))
//                 break;
//     }
//     printk("PCI: Using IRQ router %s [%04x/%04x] at %s\n",
//             pirq_router.name,
//             pirq_router_dev->vendor,
//             pirq_router_dev->device,
//             pci_name(pirq_router_dev));

//     /* The device remains referenced for the kernel lifetime */
// }

// struct pci_bus *__devinit pci_scan_bus_with_sysdata(int busno)
// {
//          struct pci_bus *bus = NULL;
//          struct pci_sysdata *sd;
 
//          /*
//           * Allocate per-root-bus (not per bus) arch-specific data.
//           * TODO: leak; this memory is never freed.
//           * It's arguable whether it's worth the trouble to care.
//           */
//          sd = kzalloc(sizeof(*sd), GFP_KERNEL);
//          if (!sd) {
//                    printk(KERN_ERR "PCI: OOM, skipping PCI bus %02x\n", busno);
//                    return NULL;
//          }
//          sd->node = -1;
//          bus = pci_scan_bus(busno, &pci_root_ops, sd);
//          if (!bus)
//                    kfree(sd);
 
//          return bus;
// }

// static void pcibios_fixup_irqs()
// {
//     struct pci_dev *dev = 0;
//     unsigned char pin;

//     printk("PCI: IRQ fixup\n");
//     while ((dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev)) != 0) {
//         /*
//         * If the BIOS has set an out of range IRQ number, just ignore it.
//         * Also keep track of which IRQ's are already in use.
//         */
//         if (dev->irq >= 16) {
//             printk("%s: ignoring bogus IRQ %d\n", pci_name(dev), dev->irq);
//             dev->irq = 0;
//         }
//         /* If the IRQ is already assigned to a PCI device, ignore its ISA use penalty */
//         if (pirq_penalty[dev->irq] >= 100 && pirq_penalty[dev->irq] < 100000)
//                 pirq_penalty[dev->irq] = 0;
//         pirq_penalty[dev->irq]++;
//     }

//     dev = 0;
//     while ((dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev)) != 0) {
//         pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
// /*
//         * Still no IRQ? Try to lookup one...
//         */
//         if (pin && !dev->irq)
//             pcibios_lookup_irq(dev, 0);
//     }
// }