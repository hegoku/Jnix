ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
CFLAGS      = -c -fno-builtin -mgeneral-regs-only -I ../include/

OBJS        = ../build/netdevice.o ../build/arp.o ../build/ip.o

all : everything

everything : net.o

# %.o : %.c
# 	$(CC) $(CFLAGS) -o ../build/$@ $<

../build/netdevice.o : netdevice.c
	$(CC) $(CFLAGS) -o $@ $<

../build/arp.o : arp.c
	$(CC) $(CFLAGS) -o $@ $<

../build/ip.o : ip.c
	$(CC) $(CFLAGS) -o $@ $<

net.o: $(OBJS)
	$(LD) -r -o ../build/$@ $(OBJS)