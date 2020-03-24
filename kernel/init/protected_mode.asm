%include "kernel/include/pm.inc"

EnterProtectedModel:
    xor eax, eax
    mov ax, ds
    shl eax, 4
    add eax, gdt_0
    mov dword [gdt_ptr+2], eax
    lgdt [gdt_ptr]

    cli

    ;开A20
    in al, 92h
    or al, 00000010b
    out 92h, al

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    mov ax, GDT_SEL_VIDEO
    mov gs, ax

    mov ax, GDT_SEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    ; mov esp, TOP_OF_KERNEL_STACK

    jmp dword GDT_SEL_CODE:(SELF_CS*10h+init32)

gdt_0: Descriptor 0, 0, 0
gdt_code: Descriptor 0, 0xfffff, (DA_CR|DA_32|DA_LIMIT_4K|DA_DPL0)
gdt_data: Descriptor 0, 0xfffff, (DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL0)
gdt_video: Descriptor 0B8000h, 0xfffff, (DA_DRW|DA_DPL3)
gdt_user_code: Descriptor 0, 0xfffff, (DA_CR|DA_32|DA_LIMIT_4K|DA_DPL3)
gdt_user_data: Descriptor 0, 0xfffff, (DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL3)
GDTLEN equ $-gdt_0
gdt_ptr dw GDTLEN-1
       dd 0

GDT_SEL_CODE equ gdt_code-gdt_0
GDT_SEL_DATA equ gdt_data-gdt_0
GDT_SEL_VIDEO equ gdt_video-gdt_0