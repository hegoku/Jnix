#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <system/tty.h>

#define STR_DEFAULT_LEN 1024

/* we use this so that we can do without the ctype library */
#define isdigit(c)	((c) >= '0' && (c) <= '9')
#define isspace(c)	((c) == ' ')

static int skip_atoi(const char **s)
{
	int i=0;

	while (isdigit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define do_div(n,base) ({ \
int __res; \
__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
__res; })

static char * number(char * str, int num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT) type &= ~ZEROPAD;
	if (base<2 || base>36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	if (type&SIGN && num<0) {
		sign='-';
		num = -num;
	} else
		sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type&SPECIAL)
		if (base==16) size -= 2;
		else if (base==8) size--;
	i=0;
	if (num==0)
		tmp[i++]='0';
	else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	if (i>precision) precision=i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type&SPECIAL)
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type&LEFT))
		while(size-->0)
			*str++ = c;
	while(i<precision--)
		*str++ = '0';
	while(i-->0)
		*str++ = tmp[i];
	while(size-->0)
		*str++ = ' ';
	return str;
}

static int vsscanf(const char *buf, const char *s, va_list ap);

static char* i2a(int val, int base, char ** ps)
{
	int m = val % base;
	int q = val / base;
	if (q) {
		i2a(q, base, ps);
	}
	*(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

	return *ps;
}

static char * _getbase(char *p, int *basep)
{
	if (p[0] == '0') {
		switch (p[1]) {
		case 'x':
			*basep = 16;
			break;
		case 't': case 'n':
			*basep = 10;
			break;
		case 'o':
			*basep = 8;
			break;
		default:
			*basep = 10;
			return (p);
		}
		return (p + 2);
	}
	*basep = 10;
	return (p);
}

static int _atob (unsigned long *vp, char *p, int base)
{
	unsigned long  value, v1, v2;
	char *q, tmp[20];
	int digit;

	if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
		base = 16;
		p += 2;
	}

	if (base == 16 && (q = strchr (p, '.')) != 0) {
		if (q - p > sizeof(tmp) - 1)
			return (0);

		strncpy (tmp, p, q - p);
		tmp[q - p] = '\0';
		if (!_atob (&v1, tmp, 16))
			return (0);

		q++;
		if (strchr (q, '.'))
			return (0);

		if (!_atob (&v2, q, 16))
			return (0);
		*vp = (v1 << 16) + v2;
		return (1);
	}

	value = *vp = 0;
	for (; *p; p++) {
		if (*p >= '0' && *p <= '9')
			digit = *p - '0';
		else if (*p >= 'a' && *p <= 'f')
			digit = *p - 'a' + 10;
		else if (*p >= 'A' && *p <= 'F')
			digit = *p - 'A' + 10;
		else
			return (0);

		if (digit >= base)
			return (0);
		value *= base;
		value += digit;
	}
	*vp = value;
	return (1);
}

static int atob(unsigned int *vp, char *p, int base)
{
	unsigned long v;

	if (base == 0)
		p = _getbase (p, &base);
	if (_atob (&v, p, base)) {
		*vp = v;
		return (1);
	}
	return (0);
}

int sprintf(char *buf, const char *format, ...)
{
	va_list arg = (va_list)((char*)(&format) + 4);        /* 4 是参数 format 所占堆栈中的大小 */
	return vsprintf(buf, format, arg);
}

int printf(const char *format, ...)
{
    int i;
    char buf[STR_DEFAULT_LEN];
    va_list arg = (va_list)((char*)(&format)+4);//4为format所占堆栈中大小
    i = vsprintf(buf, format, arg);
    write(STDOUT_FILENO, buf, i);
    return i;
}

int sscanf(const char *buf, const char *format, ...)
{
	va_list args = (va_list)((char*)(&format) + 4);
	return vsscanf(buf, format, args);
}

int scanf(const char *format, ...)
{
    int i;
    char buf[STR_DEFAULT_LEN];
    va_list arg = (va_list)((char*)(&format) + 4);
	read(STDIN_FILENO, buf, STR_DEFAULT_LEN);
    i = vsscanf (buf, format, arg);
	return i;
}

static int vsscanf (const char *buf, const char *s, va_list ap)
{
    int             count, noassign, width, base, lflag;
    const char     *tc;
    char           *t, tmp[STR_DEFAULT_LEN];

    count = noassign = width = lflag = 0;
    while (*s && *buf) {
	while (isspace (*s))
	    s++;
	if (*s == '%') {
	    s++;
	    for (; *s; s++) {
            if (strchr ("dibouxcsefg%", *s))
                break;
            if (*s == '*')
                noassign = 1;
            else if (*s == 'l' || *s == 'L')
                lflag = 1;
            else if (*s >= '1' && *s <= '9') {
                for (tc = s; isdigit (*s); s++);
                strncpy (tmp, tc, s - tc);
                tmp[s - tc] = '\0';
                atob (&width, tmp, 10);
                s--;
            }
	    }
	    if (*s == 's') {
            while (isspace (*buf))
                buf++;
            if (!width)
                width = strcspn (buf, " ");
            if (!noassign) {
                strncpy (t = va_arg (ap, char *), buf, width);
                t[width] = '\0';
            }
            buf += width;
	    } else if (*s == 'c') {
            if (!width)
                width = 1;
            if (!noassign) {
                strncpy (t = va_arg (ap, char *), buf, width);
                t[width] = '\0';
            }
            buf += width;
	    } else if (strchr ("dobxu", *s)) {
            while (isspace (*buf))
                buf++;
            if (*s == 'd' || *s == 'u')
                base = 10;
            else if (*s == 'x')
                base = 16;
            else if (*s == 'o')
                base = 8;
            else if (*s == 'b')
                base = 2;
            if (!width) {
                if (isspace (*(s + 1)) || *(s + 1) == 0)
                width = strcspn (buf, " ");
                else
                width = strchr (buf, *(s + 1)) - buf;
            }
            strncpy (tmp, buf, width);
            tmp[width] = '\0';
            buf += width;
            if (!noassign)
                atob (va_arg (ap, unsigned int *), tmp, base);
	    }
	    if (!noassign)
		count++;
	    width = noassign = lflag = 0;
	    s++;
	} else {
	    while (isspace (*buf))
		buf++;
	    if (*s != *buf)
		break;
	    else
		s++, buf++;
	}
    }
    return (count);
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	int i;
	char * str;
	char *s;
	int *ip;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;	
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			break;

		case 's':
			s = va_arg(args, char *);
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			break;

		case 'o':
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags);
			break;

		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;

		case 'n':
			ip = va_arg(args, int *);
			*ip = (str - buf);
			break;

		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			break;
		}
	}
	*str = '\0';
	return str-buf;
}

int printk(const char * format, ...)
{
    int i;
    char buf[1024];
    va_list arg = (va_list)((char*)(&format)+4);//4为format所占堆栈中大小
    i = vsprintf(buf, format, arg);
    tty_write(0, buf, i);
    return i;
}

void panic(const char * info)
{
    printk("Kernel panic: %s\n",info);
	for(;;);
}