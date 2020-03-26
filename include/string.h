#ifndef	_STRING_H_
#define	_STRING_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

// char* strcpy(char* p_dst, const char* p_src); //string.asm
// size_t strlen(const char *str); //string.asm

// void *memset(void *str, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
char *strchr(const char *str, int c);
char *strncpy(char *dest, const char *src, size_t n);
size_t strcspn(const char *str1, const char *str2);
size_t strcmp(const char * st1, const char *st2);
int memcmp(const void *str1, const void *str2, size_t n);

static inline void *memset(void *s, char c, size_t count)
{
	int d0, d1;
	asm volatile("cld\n\t"
    "rep\n\t"
		     "stosb"
		     : "=&c" (d0), "=&D" (d1)
		     : "a" (c), "1" (s), "0" (count)
		     : "memory");
	return s;
}

static inline void *memmove (void *dest, const void *src, int n)
{
	if (dest < src)
    __asm__ ("cld\n\t"		// 清方向位。
	     "rep\n\t"		// 从ds:[esi]到es:[edi]，并且esi++，edi++，
	     "movsb"		// 重复执行复制ecx 字节。
  ::"c" (n), "S" (src), "D" (dest):"cx", "si", "di");
  else
    __asm__ ("std\n\t"		// 置方向位，从末端开始复制。
	     "rep\n\t"		// 从ds:[esi]到es:[edi]，并且esi--，edi--，
	     "movsb"		// 复制ecx 个字节。
  ::"c" (n), "S" (src + n - 1), "D" (dest + n - 1):"cx", "si",
	     "di");
  return dest;
}

static inline char * strcpy(char * dest,const char *src)
{
    register char *tmp= (char *)dest;
    register char dummy;
    __asm__ __volatile__(
        "\n1:\t"
        "movb (%0),%2\n\t"
        "incl %0\n\t"
        "movb %2,(%1)\n\t"
        "incl %1\n\t"
        "testb %2,%2\n\t"
        "jne 1b"
        :"=r" (src), "=r" (tmp), "=q" (dummy)
        :"0" (src), "1" (tmp)
        :"memory");
    return dest;
}

static inline size_t strlen(const char * s)
{
    /*
    * slightly slower on a 486, but with better chances of
    * register allocation
    */
    register char dummy, *tmp= (char *)s;
    __asm__ __volatile__(
        "\n1:\t"
        "movb\t(%0),%1\n\t"
        "incl\t%0\n\t"
        "testb\t%1,%1\n\t"
        "jne\t1b"
        :"=r" (tmp),"=q" (dummy)
        :"0" (s)
        : "memory" );
    return (tmp-s-1);
}

#endif