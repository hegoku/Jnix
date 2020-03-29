ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
CFLAGS      = -c -fno-builtin -mgeneral-regs-only -I ../../include/

OBJS        = ../../build/drivers/keyboard.o ../../build/drivers/am79c793.o

all : mkdir everything

mkdir:
	mkdir -p ../../build/drivers/
	
everything : drivers/i386.o

# %.o : %.c
# 	$(CC) $(CFLAGS) -o ../build/$@ $<

../../build/drivers/keyboard.o : keyboard.c
	$(CC) $(CFLAGS) -o $@ $<

../../build/drivers/am79c793.o : am79c793.c
	$(CC) $(CFLAGS) -o $@ $<

drivers/i386.o: $(OBJS)
	$(LD) -r -o ../../build/drivers/i386.o $(OBJS)