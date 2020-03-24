ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
CFLAGS      = -c -fno-builtin -I ../include/

OBJS        = ../build/page.o ../build/kmalloc.o

all : everything

everything : mm.o

../build/page.o : page.c
	$(CC) $(CFLAGS) -o $@ $<

../build/kmalloc.o : kmalloc.c
	$(CC) $(CFLAGS) -o $@ $<

mm.o: $(OBJS)
	$(LD) -r -o ../build/$@ $(OBJS)