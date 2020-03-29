#include <stdio.h>
#include <arch/i386/init.h>
#include <system/mm.h>
#include <system/page.h>
#include <arch/i386/page.h>
#include <system/tty.h>
#include <system/thread.h>
#include <fcntl.h>

int is_in_ring0=1;
static void init();
static void init_fork();
extern struct thread *current_thread;
extern void restart();

void main()
{
    init_arch();
    init_rootfs();
    struct thread *init_p = thread_create("init", init);
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
    printk("%x\n", init);
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
    current_thread->kernel_regs.esp = __pa(current_thread->kernel_regs.esp_addr) + 1024 * 4;
    printk("%x %x %x\n", current_thread->kernel_regs.esp_addr, current_thread->kernel_regs.esp, current_thread->page_dir);
    dir->entry[511] = create_table(PG_P | PG_RWW | PG_USU);
    get_pt_entry_v_addr(dir->entry[511])->entry[1023] = get_free_page() | PG_P | PG_RWW | PG_USU;
    mem_map[(get_pt_entry_v_addr(dir->entry[511])->entry[1023] & 0xfffff000)>>12].count++;

}

__attribute((__section__(".init_process")))
static void init()
{
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