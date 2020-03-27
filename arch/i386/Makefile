ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
CFLAGS      = -c -fno-builtin -mgeneral-regs-only -I ../../include/

OBJS        = ../../build/8259A.o ../../build/idt.o ../../build/init.o ../../build/desc.o ../../build/pci.o ../../build/bios.o ../../build/timer.o \
			  ../../build/vga.o 

all : everything

everything : arch/i386.o

# %.o : %.c
# 	$(CC) $(CFLAGS) -o ../build/$@ $<

../../build/init.o : init.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/8259A.o : 8259A.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/idt.o : idt.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/desc.o : desc.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/pci.o : pci.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/bios.o : bios.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/timer.o : timer.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/vga.o : vga.c
	$(CC) $(CFLAGS) -o $@ $<

# ../../build/am79c793.o : am79c793.c
# 	$(CC) $(CFLAGS) -o $@ $<


arch/i386.o: $(OBJS)
	mkdir -p ../../build/arch
	$(LD) -r -o ../../build/arch/i386.o $(OBJS)