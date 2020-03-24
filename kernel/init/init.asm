BIOS_ADDR equ 7E00H ;bios信息存放起始地址
SELF_CS equ 0x9000 ;init程序所在的段寄存器
KERNEL_CODE_START equ 0x100000 ;内核程序加载地址
PAGE_OFFSET equ 0xC0000000

times 0x1f1 db 0
setup_sects db 2
root_flags dw 0
syssize dd 1
ram_size dw 0
vid_mode dw 0
root_dev dw 0
boot_flag dw 0xAA55
jmp start
header dd "HdrS"
version dw 0x204
realmode_swtch dd 0
start_sys_seg dw 0x1000
kernel_version dw 0
type_of_loader db 0xff
loadflags db 0000001
setup_move_size dw 0
code32_start dd 0
ramdisk_image dd 0
ramdisk_size dd 0
bootsect_kludge dd 0
heap_end_ptr dw 0x0200
ext_loader_ver db 0
ext_loader_type db 0
cmd_line_ptr dd "auto"
initrd_addr_max dd 0x37FFFFFF
; kernel_alignment dd 0
; relocatable_kernel db 0
; min_alignment db 0
; xloadflags dw 0
; cmdline_size dd 255
; hardware_subarch dd 0x00000000
; hardware_subarch_data1 dd 0
; hardware_subarch_data2 dd 0
; payload_offset dd 0
; payload_length dd 0
; setup_data1 dd 0
; setup_data2 dd 0
; pref_address1 dd 0
; pref_address2 dd 0
; init_size dd 0
; handover_offset dd 0

[BITS 16]
start:
    call ClearScreen

    mov ax, reading_mem_msg
    call DispStr
    call ReadMemorySize

    mov ax, setting_gdt_msg
    call DispStr
    call ClearScreen
    jmp EnterProtectedModel

DispStr: ;显示字符串, ax作为字符串地址
    ; push es
    push bx
    mov bp, ax
    ; mov ax, REAL_MODE_SEG
    ; mov es, ax
    mov cx, 16
    mov ax, 01301h
    mov bx, 0007h
    mov dl, 0
    mov dh, [print_line]
    int 10h
    inc dh
    mov [print_line], dh
    pop bx
    ; pop es
    ret

ClearScreen:
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0
    mov dx, 0184fh
    int 10h
    mov ax, 0200h
    mov bh, 0
    mov dx, 0
    int 10h
    ret

print_line db 0 ;下一次打印的行数
reading_mem_msg db 'Reading Memory  '
setting_gdt_msg db 'Setting GDT     '

%include "kernel/init/memory.asm"
%include "kernel/init/protected_mode.asm"
%include "kernel/init/page.asm"

[BITS 32]
init32:
    call EnablePaging
    jmp dword GDT_SEL_CODE:(KERNEL_CODE_START+PAGE_OFFSET)
    
times 1536-($-$$) db 0