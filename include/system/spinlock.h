#ifndef _SYSTEM_SPINLOCK_H
#define _SYSTEM_SPINLOCK_H

typedef struct {
    volatile unsigned int lock;
} raw_spinlock_t;

typedef struct {
    raw_spinlock_t raw_lock;
} spinlock_t;

static inline void __raw_spin_lock(raw_spinlock_t *lock)
{
    asm volatile("/n1:/t"
             "lock ; decb %0/n/t"
             "jns 3f/n"
             "2:/t"
             "rep;nop/n/t"
             "cmpb $0,%0/n/t"
             "jle 2b/n/t"
             "jmp 1b/n"
             "3:/n/t"
             : "+m" (lock->lock) : : "memory");
}

static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
    asm volatile("movb $1,%0" : "+m" (lock->lock) :: "memory");
}
#endif