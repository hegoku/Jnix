#include <arch/i386/pci.h>
#include <drivers/i386/am79c793.h>
#include <arch/i386/8295A.h>
#include <arch/i386/io.h>
#include <arch/i386/idt.h>
#include <system/mm.h>
#include <system/page.h>
#include <string.h>
#include <stdio.h>
#include <net/netdevice.h>
#include <system/init.h>
#include <net/net.h>

int rx_buffer_count = 32;              // total number of receive buffers
int tx_buffer_count = 8;               // total number of transmit buffers
unsigned int buffer_size = 1544;

static void __attribute__((interrupt)) am79c_interrupt_handle(struct interrupt_frame *frame);
void receive(struct net_device *dev);

int am79c793_probe(struct net_device *dev)
{
    PCI_DEVICE_DESC *pdev;
    pdev = pci_find_device(0x1022, 0x2000);
    if (pdev==0) {
        return -1;
    }

    printk("am79c793 found int:%x, ", pdev->interrupt_line);

    PCI_DEVICE_BA *next = pdev->ba;
    while (next->next!=0)
    {
        if (next->next->map_type == 1)
        {
            dev->io_base = next->next->address;
            break;
        }
        next = next->next;
    }

    dev->irq = pdev->interrupt_line;
    return 0;
}

int init(struct net_device *dev)
{
    if (am79c793_probe(dev)==-1) {
        return -1;
    }
    register_interrupt(INT_VECTOR_IRQ0 + dev->irq, am79c_interrupt_handle, dev);
    enable_8259A_irq(dev->irq);

    // //reset
    // inw(io_base+AM79C79C_RESET_PORT);
    // outw(0, io_base+AM79C79C_RESET_PORT);

    unsigned int mac0 = inw(dev->io_base + AM79C79C_MAC0_PORT)%256;
    unsigned int mac1 = inw(dev->io_base + AM79C79C_MAC0_PORT)/256;
    unsigned int mac2 = inw(dev->io_base + AM79C79C_MAC2_PORT)%256;
    unsigned int mac3 = inw(dev->io_base + AM79C79C_MAC2_PORT)/256;
    unsigned int mac4 = inw(dev->io_base + AM79C79C_MAC4_PORT)%256;
    unsigned int mac5 = inw(dev->io_base + AM79C79C_MAC4_PORT)/256;

    unsigned int mac_low = (mac2 << 24) | (mac3 << 16) | (mac4 << 8) | mac5;
    unsigned int mac_high = (mac0 << 8) | (mac1);
    // unsigned long mac = (mac5 << 40) | (mac4 << 32) | (mac3 << 24) | (mac2 << 16) | (mac1 << 8) | mac0;
    // printk(" I/O port:%x MAC:%x:%x:%x:%x:%x:%x\n", io_base, mac0,mac1,mac2,mac3,mac4,mac5);
    printk("I/O port:%x MAC:%x%x\n", dev->io_base, mac_high, mac_low);

    //32bit mode
    outw(20, dev->io_base + AM79C79C_ADD_PORT);
    outw(0x102, dev->io_base + AM79C79C_BUSREG_PORT);

    //STOP reset
    outw(0, dev->io_base + AM79C79C_ADD_PORT);
    outw(0x04, dev->io_base + AM79C79C_DATA_PORT);

    dev->custom_data = kzmalloc(sizeof(struct am79c793_data));

    TO_AMDATA(dev->custom_data)->init_block.mode = 0x0000; // promiscuous mode = false
    // initBlock->reserved1 = 0;
    TO_AMDATA(dev->custom_data)->init_block.rlen = 5<<4; //2^5=rx_buffer_count
    // initBlock->reserved2 = 0;
    TO_AMDATA(dev->custom_data)->init_block.tlen = 3<<4; //2^3=tx_buffer_count
    // initBlock->mac_low = mac3 << 24 | mac2 << 16 | mac1 << 8 | mac0;
    // initBlock->mac_high = mac5 << 8 | mac4;
    TO_AMDATA(dev->custom_data)->init_block.mac[0] = mac0;
    TO_AMDATA(dev->custom_data)->init_block.mac[1] = mac1;
    TO_AMDATA(dev->custom_data)->init_block.mac[2] = mac2;
    TO_AMDATA(dev->custom_data)->init_block.mac[3] = mac3;
    TO_AMDATA(dev->custom_data)->init_block.mac[4] = mac4;
    TO_AMDATA(dev->custom_data)->init_block.mac[5] = mac5;
    memcpy(dev->dev_addr, TO_AMDATA(dev->custom_data)->init_block.mac, 6);
    TO_AMDATA(dev->custom_data)->init_block.reserved3 = 0;
    TO_AMDATA(dev->custom_data)->init_block.ladr_low = 0;
    TO_AMDATA(dev->custom_data)->init_block.ladr_hight = 0;

    TO_AMDATA(dev->custom_data)->sendBufferDescr = kzmalloc(sizeof(struct buffer_descriptor) * tx_buffer_count);
    TO_AMDATA(dev->custom_data)->recvBufferDescr = kzmalloc(sizeof(struct buffer_descriptor) * rx_buffer_count);
    TO_AMDATA(dev->custom_data)->init_block.recvBufferDescrAddress = __pa(TO_AMDATA(dev->custom_data)->recvBufferDescr);
    TO_AMDATA(dev->custom_data)->init_block.sendBufferDescrAddress = __pa(TO_AMDATA(dev->custom_data)->sendBufferDescr);

    for(unsigned int i = 0; i <tx_buffer_count ; i++)   
    {
        TO_AMDATA(dev->custom_data)->sendBufferDescr[i].address = (void*)__pa(kzmalloc(buffer_size));
        TO_AMDATA(dev->custom_data)->sendBufferDescr[i].flags = 0x7FF | 0xF000;
        TO_AMDATA(dev->custom_data)->sendBufferDescr[i].flags2 = 0;
        TO_AMDATA(dev->custom_data)->sendBufferDescr[i].avail = 0;
    }

    for(unsigned int i = 0; i <rx_buffer_count ; i++)
    {
        TO_AMDATA(dev->custom_data)->recvBufferDescr[i].address = (void*)__pa(kzmalloc(buffer_size));
        TO_AMDATA(dev->custom_data)->recvBufferDescr[i].flags = 0xF7FF | 0x80000000;
        TO_AMDATA(dev->custom_data)->recvBufferDescr[i].flags2 = 0;
        TO_AMDATA(dev->custom_data)->recvBufferDescr[i].avail = 0;
    }

    outw(1, dev->io_base + AM79C79C_ADD_PORT);
    outw((unsigned int)(__pa(&(TO_AMDATA(dev->custom_data)->init_block))) & 0xFFFF, dev->io_base + AM79C79C_DATA_PORT);
    outw(2, dev->io_base + AM79C79C_ADD_PORT);
    outw(( (unsigned int)__pa( &(TO_AMDATA(dev->custom_data)->init_block)) >>16) & 0xFFFF, dev->io_base + AM79C79C_DATA_PORT);

    //active
    outw(0, dev->io_base + AM79C79C_ADD_PORT);
    outw(0x41, dev->io_base + AM79C79C_DATA_PORT);

    outw(4, dev->io_base + AM79C79C_ADD_PORT);
    unsigned int temp = inw(dev->io_base + AM79C79C_DATA_PORT);
    outw(4, dev->io_base + AM79C79C_ADD_PORT);
    outw(temp | 0xC00, dev->io_base + AM79C79C_DATA_PORT);

    outw(0, dev->io_base + AM79C79C_ADD_PORT);
    outw(0x42, dev->io_base + AM79C79C_DATA_PORT);

    
    // outw(18, io_base + AM79C79C_ADD_PORT);
    // printk("bb:%x\n", inw(io_base + AM79C79C_DATA_PORT));
    // outw(19, io_base + AM79C79C_ADD_PORT);
    // printk("bb:%x\n", inw(io_base + AM79C79C_DATA_PORT));
    // outw(20, io_base + AM79C79C_ADD_PORT);
    // printk("bb:%x\n", inw(io_base + AM79C79C_DATA_PORT));
    // outw(21, io_base + AM79C79C_ADD_PORT);
    // printk("bb:%x\n", inw(io_base + AM79C79C_DATA_PORT));
    return 0;
}

void receive(struct net_device *dev)
{
    while((TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].flags & 0x80000000) == 0) {
        if(!(TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].flags & 0x40000000)
         && (TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].flags & 0x03000000) == 0x03000000) 
        
        {
            unsigned int size = TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].flags2 & 0xFFF;
            // if(size > 64) // remove checksum
            //     size -= 4;
            unsigned char* buffer = (unsigned char*)(__va(TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].address));

            // for(int i = 14+20; i < (size>64?64:size); i++)
            for(int i = 0; i < size; i++)
            {
                printk("%02x ", buffer[i]);
            }
            printk("\n");
            reveice_callback(buffer, size-4, dev); //去掉最后4字节的CRC校验码
            // if(handler != 0)
            //     if(handler->OnRawDataReceived(buffer, size))
            //         Send(buffer, size);
        }
        
        TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].flags2 = 0;
        TO_AMDATA(dev->custom_data)->recvBufferDescr[TO_AMDATA(dev->custom_data)->current_rx_buffer].flags = 0x8000F7FF;
        if (TO_AMDATA(dev->custom_data)->current_rx_buffer+1>rx_buffer_count) {
            TO_AMDATA(dev->custom_data)->current_rx_buffer = 0;
        } else {
            TO_AMDATA(dev->custom_data)->current_rx_buffer++;
        }
    }
}

int send(struct net_device *dev, unsigned char* packet, int size)
{
    // int sendDescriptor = currentSendBuffer;
    // currentSendBuffer = (currentSendBuffer + 1) % 8;
    if ((TO_AMDATA(dev->custom_data)->sendBufferDescr[TO_AMDATA(dev->custom_data)->current_tx_buffer].flags & 0x80000000)!=0) {
        printk("tx_drop %d %x\n",TO_AMDATA(dev->custom_data)->current_tx_buffer,TO_AMDATA(dev->custom_data)->sendBufferDescr[TO_AMDATA(dev->custom_data)->current_tx_buffer].flags);
        dev->tx_dropped++;
        return -1;
    }
    if(size > buffer_size) {
        return -1;
    }

    memcpy((void*)__va(TO_AMDATA(dev->custom_data)->sendBufferDescr[TO_AMDATA(dev->custom_data)->current_tx_buffer].address), packet, size);

    // for(unsigned char *src = buffer + size -1,
    //             *dst = (unsigned char*)(sendBufferDescr[sendDescriptor].address + size -1);
    //             src >= buffer; src--, dst--)
    //     *dst = *src;
        
    // printk("SEND: %d %x\n",current_tx_buffer,sendBufferDescr[current_tx_buffer].flags);
    // for(int i = 14+20; i < (size>64?64:size); i++)
    for(int i = 0; i < (size); i++)
    {
        printk("%02x ", packet[i]);
        // printfHex(buffer[i]);
        // printf(" ");
    }
    printk("\n");

    TO_AMDATA(dev->custom_data)->sendBufferDescr[TO_AMDATA(dev->custom_data)->current_tx_buffer].avail = 0;
    TO_AMDATA(dev->custom_data)->sendBufferDescr[TO_AMDATA(dev->custom_data)->current_tx_buffer].flags2 = 0;
    TO_AMDATA(dev->custom_data)->sendBufferDescr[TO_AMDATA(dev->custom_data)->current_tx_buffer].flags = 0x8300F000 | ((unsigned short)((-size) & 0xFFF));
    outw(0, dev->io_base + AM79C79C_ADD_PORT);
    outw(0x48, dev->io_base + AM79C79C_DATA_PORT);

    if (TO_AMDATA(dev->custom_data)->current_tx_buffer+1>=tx_buffer_count) {
        TO_AMDATA(dev->custom_data)->current_tx_buffer = 0;
    } else {
        TO_AMDATA(dev->custom_data)->current_tx_buffer++;
    }
    return 0;
}

int header_ops_create(struct ethhdr *eth, struct net_device *dev, unsigned short type, const void *daddr, const void *saddr)
{
    // if (len<sizeof(struct ethhdr)) {
    //     return -1;
    // }
    // struct ethhdr **eth = buffer;
    // memcpy((*eth)->h_dest, daddr, sizeof((*eth)->h_dest));
    // if (daddr) {
    //     memcpy((*eth)->h_source, daddr, sizeof((*eth)->h_source));
    // } else {
    //     memcpy((*eth)->h_source, TO_AMDATA(dev->custom_data)->init_block.mac, sizeof((*eth)->h_source));
    // }
    // (*eth)->h_proto = type;
    // buffer->len = sizeof(struct ethhdr);
    // return 0;
    memcpy(eth->h_dest, daddr, sizeof(eth->h_dest));
    if (saddr) {
        memcpy(eth->h_source, saddr, sizeof(eth->h_source));
    } else {
        memcpy(eth->h_source, dev->dev_addr, sizeof(eth->h_source));
    }
    eth->h_proto = htons(type);
    return 0;
}

struct net_device am79c793 = {
    name:"AM79C793",
    init:init,
    header_ops_create:header_ops_create,
    send:send
};

static void __attribute__ ((interrupt)) am79c_interrupt_handle(struct interrupt_frame *frame)
{
    disable_8259A_irq(am79c793.irq);
    // eoi_8259A();
    // printk("a\n");
    outw(0, am79c793.io_base + AM79C79C_ADD_PORT);
    unsigned int temp = inw(am79c793.io_base + AM79C79C_DATA_PORT);

    if((temp & 0x8000) == 0x8000) printk("AMD am79c973 ERROR\n");
    if((temp & 0x2000) == 0x2000) printk("AMD am79c973 COLLISION ERROR\n");
    if((temp & 0x4000) == 0x4000) {
        am79c793.tx_dropped++;
        printk("AMD am79c973 tx_error\n");
    }
    if((temp & 0x1000) == 0x1000) {
        am79c793.rx_dropped++;
        printk("AMD am79c973 MISSED FRAME\n");
    }
    if((temp & 0x0800) == 0x0800) printk("AMD am79c973 MEMORY ERROR\n");
    if((temp & 0x0400) == 0x0400) receive(&am79c793);
    if((temp & 0x0200) == 0x0200) printk("AMD am79c973 SENT\n");
                               
    // acknoledge
    outw(0, am79c793.io_base + AM79C79C_ADD_PORT);
    outw(temp, am79c793.io_base + AM79C79C_DATA_PORT);
    
    if((temp & 0x0100) == 0x0100) printk("AMD am79c973 INIT DONE\n");
    enable_8259A_irq(am79c793.irq);
}

static int am79c793_init()
{
    register_netdev(&am79c793);
    return 0;
}

module_init(am79c793_init);