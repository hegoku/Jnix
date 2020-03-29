ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
OBJDUMP     = i386-elf-objdump
CFLAGS      = -c -fno-builtin -mgeneral-regs-only -I ../include/

OBJS        = ../build/file_op.o ../build/super.o

all : everything

everything : fs.o

../build/file_op.o : file_op.c
	$(CC) $(CFLAGS) -o $@ $<

../build/super.o : rootfs/super.c
	$(CC) $(CFLAGS) -o $@ $<

fs.o: $(OBJS)
	$(LD) -r -o ../build/$@ $(OBJS)