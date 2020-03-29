#include <stdio.h>
#include <arch/i386/init.h>
#include <system/mm.h>
#include <system/tty.h>

void main()
{
    init_arch();
    init_tty();
    while(1){}
    char *a = kzmalloc(256);
    printk("%x\n", a);
    // while(1){}
    a[0] = 'A';
    a[1] = '0';
    a[2] = '\n';
    printk("%s\n", a);
    while(1){}
}