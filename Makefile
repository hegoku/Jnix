ENTERPOINT  = 0xC0100000

ASM			= nasm
CC          = i386-elf-gcc
LD			= i386-elf-ld
OBJCOPY     = i386-elf-objcopy
OBJDUMP     = i386-elf-objdump
CFLAGS      = -c -fno-builtin -I include/

SUBDIR=lib mm arch/i386 net drivers/i386
BUILDSUBDIR = $(SUBDIR:%=build-%)
CLEANSUBDIR = $(SUBDIR:%=clean-%)
OBJSUBDIR = $(SUBDIR:%=build/%.o)

.PHONY: all $(SUBDIR) $(BUILDSUBDIR) $(CLEANSUBDIR)

OBJS        =  build/kernel_asm.o build/kernel_c.o build/tty.o build/interrupt.o

all : clean everything buildimg debug

everything : init.bin kernel.bin 

clean :
	rm -Rf build/*

debug:
	$(OBJDUMP) -j .text -l -d -r build/kernel.elf >build/kernel.elf.txt

buildimg:
	cat build/init.bin build/kernel.bin > build/init
	hdiutil attach -imagekey diskimage-class=CRawDiskImage c1.img
	cp build/init /Volumes/NO\ NAME/init
	hdiutil detach /Volumes/NO\ NAME/
	rm -f c1.vdi
	VBoxManage convertfromraw --format VDI c1.img c1.vdi
	vboxmanage storageattach orange --storagectl "IDE" --port 0 --device 0 --medium emptydrive
	vboxmanage closemedium  disk c1.vdi
	vboxmanage storageattach orange --storagectl "IDE" --port 0 --device 0 --medium c1.vdi --type hdd

init.bin : kernel/init/init.asm kernel/init/memory.asm kernel/init/protected_mode.asm kernel/init/page.asm
	$(ASM)  -o build/$@ $<

kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary build/kernel.elf build/kernel.bin
#$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/kernel.elf $(OBJS) $(OBJSUBDIR)
build/kernel.elf : $(OBJS) $(BUILDSUBDIR)
	$(LD) -s -Tjnix.ls.S -m elf_i386 -o build/kernel.elf $(OBJS) $(OBJSUBDIR)

build/kernel_asm.o : kernel/kernel.asm
	$(ASM) -f elf -o $@ $<

build/kernel_c.o : kernel/kernel.c
	$(CC) $(CFLAGS) -o $@ $<

build/interrupt.o : kernel/interrupt.c
	$(CC) $(CFLAGS) -o $@ $<

build/tty.o : kernel/tty.c
	$(CC) $(CFLAGS) -o $@ $<

build/spinlock.o : kernel/spinlock.c
	$(CC) $(CFLAGS) -o $@ $<

#arch
# build/arch_i386.o: arch/i386/init.c
# 	$(CC) $(CFLAGS) -o $@ $<

$(BUILDSUBDIR):
	@echo "===>" $@
		make -C $(@:build-%=%)
	@echo "<===" $@