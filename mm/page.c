#include "system/page.h"
#include "stdio.h"
#include "string.h"

struct Page *mem_map=0;
unsigned int mem_map_count = 0;

//返回一个未使用的页的物理地址, 即物理页地址
int get_free_page()
{
    for (int i = 0; i < mem_map_count;i++) {
        if ((mem_map[i].flags & MP_USE) == 0)
        {
            mem_map[i].flags |= MP_USE;
            mem_map[i].count = 1;
            memset((void *)__va(i << PAGE_SHIFT), 0, PAGE_SIZE);
            return i << PAGE_SHIFT;
        }
    }
    panic("Not more page.\n");
    return -1;
}

//将物理页释放
void free_page(unsigned int pyhsics_addr)
{
    pyhsics_addr >>= PAGE_SHIFT;
    mem_map[pyhsics_addr].flags = ~MP_USE;
    mem_map[pyhsics_addr].count--;
}