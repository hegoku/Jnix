#ifndef _SYSTEM_INIT_H
#define _SYSTEM_INIT_H

#define __section(sec) __attribute__((__section__("sec")))

#define __init		__section(.init.text)
#define __initdata	__section(.init.data)
#define __initconst	__constsection(.init.rodata)
#define __exitdata	__section(.exit.data)
#define __exit_call	__used __section(.exitcall.exit)

#define module_init(x)  __initcall(x);

#define __initcall(fn) device_initcall(fn)
#define core_initcall(fn)   __define_initcall(fn, 1)
#define device_initcall(fn) __define_initcall(fn, 6)

#define __define_initcall(fn, id) \
                static initcall_t __initcall_##fn##id  \
                __attribute__((__section__(".initcall" #id ".init"))) = fn

typedef int (*initcall_t)(void);

extern initcall_t __initcall_start[];
extern initcall_t __initcall0_start[];
extern initcall_t __initcall1_start[];
extern initcall_t __initcall2_start[];
extern initcall_t __initcall3_start[];
extern initcall_t __initcall4_start[];
extern initcall_t __initcall5_start[];
extern initcall_t __initcall6_start[];
extern initcall_t __initcall7_start[];
extern initcall_t __initcall_end[];

static initcall_t *initcall_levels[] __initdata = {
    __initcall0_start,
    __initcall1_start,
    __initcall2_start,
    __initcall3_start,
    __initcall4_start,
    __initcall5_start,
    __initcall6_start,
    __initcall7_start,
    __initcall_end,
};

static void do_initcalls()
{
    int level;
    for (level = 0; level < 9 - 1; level++) {
        initcall_t *fn;
        for (fn = initcall_levels[level]; fn < initcall_levels[level+1]; fn++) {
            (*fn)();
        }
    }
}
#endif