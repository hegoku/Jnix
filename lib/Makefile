ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
CFLAGS      = -c -fno-builtin -I ../include/

OBJS        = ../build/stdio.o ../build/stdlib.o ../build/unistd.o ../build/string.o ../build/math.o

all : everything

everything : lib.o

# %.o : %.c
# 	$(CC) $(CFLAGS) -o ../build/$@ $<

../build/stdio.o : stdio.c
	$(CC) $(CFLAGS) -o $@ $<

../build/stdlib.o : stdlib.c
	$(CC) $(CFLAGS) -o $@ $<

../build/unistd.o : unistd.c
	$(CC) $(CFLAGS) -o $@ $<

../build/string.o : string.c
	$(CC) $(CFLAGS) -o $@ $<

../build/math.o : math.c
	$(CC) $(CFLAGS) -o $@ $<

lib.o: $(OBJS)
	$(LD) -r -o ../build/$@ $(OBJS)