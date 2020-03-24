#ifndef _SYSTEM_MM_H
#define _SYSTEM_MM_H

void *kmalloc(unsigned int len);
void *kzmalloc(unsigned int size);
void kfree(void *obj, unsigned int size);

#endif