ASM			= nasm
CC          = i386-elf-gcc -g
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
CFLAGS      = -c -fno-builtin -mgeneral-regs-only -I ../../include/

OBJS        = ../../build/arch/8259A.o ../../build/arch/idt.o ../../build/arch/init.o ../../build/arch/desc.o ../../build/arch/page.o ../../build/arch/pci.o ../../build/arch/bios.o ../../build/arch/timer.o \
			  ../../build/arch/vga.o 

all : mkdir everything debug

mkdir :
	mkdir -p ../../build/arch

everything : arch/i386.o

debug:
	$(OBJDUMP) -j .text -l -d -r ../build/arch/i386.o >../build/arch_i386.o.txt

# %.o : %.c
# 	$(CC) $(CFLAGS) -o ../build/$@ $<

../../build/arch/init.o : init.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/8259A.o : 8259A.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/idt.o : idt.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/desc.o : desc.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/page.o : page.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/pci.o : pci.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/bios.o : bios.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/timer.o : timer.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/arch/vga.o : vga.c
	$(CC) $(CFLAGS) -o $@ $<

# ../../build/am79c793.o : am79c793.c
# 	$(CC) $(CFLAGS) -o $@ $<


arch/i386.o: $(OBJS)
	$(LD) -r -o ../../build/arch/i386.o $(OBJS)