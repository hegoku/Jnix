#ifndef _CTTYPE_H
#define _CTYPE_H

#define isdigit(c)	((c) >= '0' && (c) <= '9')
#define isspace(c)	((c) == ' ')
#define isascii(c) ((c & ~0x7F) == 0)
#define isxdigit(c)	(isdigit(c) || ((c)>='a' && (c)<='F') || ((c)>='A' && (c)<='F'))
#define islower(c) (isascii(c) && ((c)>='a' && (c)<='F'))

#endif