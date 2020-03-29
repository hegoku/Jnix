ASM			= nasm
CC          = i386-elf-gcc -g
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
OBJDUMP     = i386-elf-objdump
CFLAGS      = -c -fno-builtin -mgeneral-regs-only -I ../include/

OBJS        = ../build/netdevice.o ../build/arp.o ../build/ip.o ../build/icmp.o ../build/skbuffer.o

all : everything debug

everything : net.o

debug:
	$(OBJDUMP) -j .text -l -d -r ../build/net.o >../build/net.o.txt

# %.o : %.c
# 	$(CC) $(CFLAGS) -o ../build/$@ $<

../build/netdevice.o : netdevice.c
	$(CC) $(CFLAGS) -o $@ $<

../build/arp.o : arp.c
	$(CC) $(CFLAGS) -o $@ $<

../build/ip.o : ip.c
	$(CC) $(CFLAGS) -o $@ $<

../build/icmp.o : icmp.c
	$(CC) $(CFLAGS) -o $@ $<

../build/skbuffer.o : skbuffer.c
	$(CC) $(CFLAGS) -o $@ $<

net.o: $(OBJS)
	$(LD) -r -o ../build/$@ $(OBJS)