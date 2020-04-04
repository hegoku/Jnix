#include <stdio.h>
#include <arch/i386/init.h>
#include <system/mm.h>
#include <system/page.h>
#include <arch/i386/page.h>
#include <system/tty.h>
#include <system/thread.h>
#include <fcntl.h>
#include <system/init.h>

#include <net/skbuffer.h>
#include <net/netdevice.h>
#include <system/init.h>
#include <net/net.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <drivers/i386/am79c793.h>
#include <net/ether.h>
#include <net/arp.h>

int is_in_ring0=1;
static void init();
static void init_fork();
extern struct thread *current_thread;
extern void restart();

static void handle_net();

void main()
{
    init_arch();
    init_rootfs();
    do_initcalls();
    asm("cli");
    struct thread *init_p = thread_create("init", init, THREAD_TYPE_KERNEL);
    // init_tty();
    char *a = kzmalloc(256);
    printk("%x\n", a);
    // while(1){}
    a[0] = 'A';
    a[1] = '0';
    a[2] = '\n';
    printk("%s\n", a);

    init_fork();
    is_in_ring0 = 0;
    load_cr3(__pa(init_p->page_dir));
    // printk("%x\n", init);
    // clear_screen(&tty_table[0]);
    restart();
}

static void init_fork()
{
    current_thread->page_dir = create_dir();
    struct PageDir *dir = (current_thread->page_dir);
    int i = 0;
    int j = 0;
    for (int i = 768; i < 1024; i++)
    {
        dir->entry[i]= ((struct PageDir *)__va(PAGE_DIR_BASE))->entry[i];
    }

    current_thread->kernel_regs.esp_addr=__va(get_free_page());
    current_thread->kernel_regs.esp = (current_thread->kernel_regs.esp_addr) + 1024 * 4;
    printk("%x %x %x %x\n", current_thread->kernel_regs.esp_addr, current_thread->kernel_regs.esp, current_thread->page_dir, current_thread->regs.esp);
    dir->entry[511] = create_table(PG_P | PG_RWW | PG_USS);
    get_pt_entry_v_addr(dir->entry[511])->entry[1023] = get_free_page() | PG_P | PG_RWW | PG_USS;
    mem_map[(get_pt_entry_v_addr(dir->entry[511])->entry[1023] & 0xfffff000)>>12].count++;

}

int init1_fork()
{
    int i;
    struct thread *p=thread_create("", handle_net, current_thread->flag);

    *p=*current_thread;
    p->parent_pid=current_thread->pid;
	sprintf(p->name, "%s_%d", current_thread->name, p->pid);
	p->regs.eax = 0;
	p->status = TASK_RUNNING;
    p->page_dir = create_dir();

    copy_page(p->page_dir, &(current_thread->page_dir));

    //文件描述符
    for (i = 0; i < PROC_FILES_MAX_COUNT; i++)
    {
        if (p->file_table[i]) {
            p->file_table[i]->inode->used_count++;
            p->file_table[i]->used_count++;
        }
    }
    p->regs.eip=handle_net;
    p->kernel_regs.esp_addr = __va(get_free_page());
    p->kernel_regs.esp = (p->kernel_regs.esp_addr) + 1024 * 4;

    // printk("fork pid(%d): %x %x eip:%x esp:%x\n", current_process->pid, process_table[pid].kernel_regs.esp_addr, process_table[pid].kernel_regs.esp, current_process->regs.eip, current_process->regs.esp);
    return p->pid;
}

static void init()
{
    printk("333\n");
    // init1_fork();
    while(1){}

    am79c793.ip=inet_aton("10.0.2.12");
    // unsigned char *mac=0;
    // unsigned char null_mac[]={'0', '0', '0', '0', '0', '0'};
    int a=0;
    unsigned char *mac = 0;
    while (mac == 0)
    {
        mac=arp_find(&am79c793, inet_aton("10.0.2.15"));
        if (mac==0) {
            arp_send(ARPOP_REQUEST, ETH_P_ARP, htonl(inet_aton("10.0.2.15")), &am79c793, htonl(am79c793.ip), 0, 0, 0);
        }
        // a=mac[0]+mac[1]+mac[2]+mac[3]+mac[4]+mac[5];
    }
    // icmp_echo(&am79c793, 1, 1, inet_aton("10.0.2.4"));
    // icmp_timestamp(&am79c793, 1, 1, inet_aton("10.0.2.4"), 0);
    icmp_hostano(&am79c793, 0);
    while(1){}
    mount("/dev/hda3", "/root", "fat");
    // sys_mkdir("/root/dev", 1);
    // mount("/dev", "/root/dev", "devfs");
    (void) open("/dev/tty0", O_RDWR);
    (void) dup(0);
    (void) dup(0);
    chroot("/root");
    chdir("/root");
    printf("================================================================================\n");
    printf("                  -----    |\\   |   ---    \\  /       \n");
    printf("                    |      | \\  |    |      \\/         \n");
    printf("                    |      |  \\ |    |      /\\         \n");
    printf("                 ___|      |   \\|   ---    /  \\     Powered by Jack He   \n");
    printf("================================================================================\n");
    int i, pid;
    while(1){}
    // pid = fork();
    char buf[1024];
    if (pid != 0)
    {
        // printf("parent is running,child pid: %d %d %d %d\n", pid, getpid(), getppid());
    } else {
        // printf("childis running %d\n", getpid());
        // pid = fork();
        // if (pid) {
        //     printf("chchchchh %d\n", pid);
        // }
        // else
        // {
        //     printf("ggggggg %d\n", getpid());
        //     exit(2);
        // }
        // execve("/sh", NULL, NULL);
        // exit(4);
    }
    // while(pid=waitpid(-1, &i, 0)) {
    //     if (pid>0) {
    //         printf("child: %d exit with status: %d\n", pid, i);
    //     }
    // }
}

static void handle_net()
{
    printk("11\n");
    while(1){
        // skb_handle_recv();
    }
}