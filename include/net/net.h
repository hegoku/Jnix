#ifndef _NET_NET_H
#define _NET_NET_H

struct sk_buffer {
    unsigned char *data;
    unsigned int len;
};
// static inline unsigned short swap_uint16(unsigned short val) {
//     return (val << 8) | (val >> 8);
// }

// static inline short swap_int16(short val) {
//     return (val << 8) | ((val >> 8) & 0xFF);
// }

// static inline unsigned int swap_uint32(unsigned int val) {
//     val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
//     return (val << 16) | (val >> 16);
// }

// static inline int swap_int32(int val) {
//     val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
//     return (val << 16) | ((val >> 16) & 0xFFFF);
// }

static inline int check_cpu_endian()
{
    union {
        unsigned int i;
        unsigned char s[4];
    } c;
    c.i = 0x12345678;
    return (0x12 == c.s[0]);
}

static inline unsigned int htonl(unsigned int h)
{
    h = ((h << 8) & 0xFF00FF00) | ((h >> 8) & 0xFF00FF);
    return (h << 16) | (h >> 16);
    // return check_cpu_endian() ? h : swap_uint32(h);
}

static inline unsigned int ntohl(unsigned int n)
{
    n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0xFF00FF);
    return (n << 16) | (n >> 16);
    // return check_cpu_endian() ? n : swap_uint32(n);
}

static inline unsigned short htons(unsigned short h)
{
    return (h << 8) | (h >> 8);
    // return check_cpu_endian ? h : swap_uint16(h);
}

static inline unsigned short ntohs(unsigned short n)
{
    return (n << 8) | (n >> 8);
    // return check_cpu_endian() ? n : swap_uint16(n);
}

#endif