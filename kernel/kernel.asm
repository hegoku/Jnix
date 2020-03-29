global _start
global save
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15

extern main
extern tss
extern is_in_ring0
extern current_thread
extern irq_table

PAGE_OFFSET equ 0xC0000000
STACKTOP equ 0x7e00

%include "kernel/include/pm.inc"

[SECTION .data]
disp_pos dd	0

[SECTION .text]
[BITS 32]

_start:
    mov esp, STACKTOP+PAGE_OFFSET
    call main
    jmp $

DispStr:
    push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi
    ;移动光标
    ; cli
    xor eax, eax
    xor edx, edx
    mov edx, 0x3d4
    mov al, 0xe
    out dx, al
    nop
    nop
    mov eax, [disp_pos]
    shr eax, 1
    shr eax, 8
    mov edx, 0x3d5
    out dx, al
    nop
    nop
    mov al, 0xf
    mov edx, 0x3d4
    out dx, al
    nop
    nop
    mov eax, [disp_pos]
    shr eax, 1
    mov edx, 0x3d5
    out dx, al
    nop
    nop
    sti

	pop	ebp
	ret

save:
    pushad
    push ds
    push es
    push fs
    push gs

    mov esi,ss
    mov ds, esi
    mov es,esi

    mov esi, esp ;进程表起始地址

    inc dword[is_in_ring0]
    cmp dword[is_in_ring0], 0
    jne .1
    mov esp, [current_thread]
    mov esp, [esp+P_K_ESP]
    push restart
    jmp [esi+RETADR-P_STACKBASE]
.1:
    push ret_to_proc
    jmp [esi+RETADR-P_STACKBASE]

restart:
    mov	esp, [current_thread]
    mov eax, [esp+P_K_ESP_ADDR]
    add eax, 1024*4
    mov dword[esp+P_K_ESP], eax ;重新设置进程内核态栈顶, 否则switch_to要堆栈溢出
	lldt [esp + P_LDT_SEL]
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax
ret_to_proc:
    dec dword[is_in_ring0]
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp,4
    iretd

%macro	hwint_master 1
    call save

    in al, INT_M_CTLMASK ;屏蔽当前中断
    or al, (1<<%1)
    out INT_M_CTLMASK, al

    push dword[(irq_table+4*3*%1)+4*2] ;struct interrupt.dev_id
    push %1
	call [(irq_table+4*3*%1)+4] ;struct interrupt.handler
	add	esp, 4*2

    in al, INT_M_CTLMASK ;恢复当前中断
    and al, ~(1<<%1)
    out INT_M_CTLMASK, al

	ret
%endmacro

%macro	hwint_slave	1
	call save

    in al, INT_S_CTLMASK ;屏蔽当前中断
    or al, (1<<(%1-8))
    out INT_S_CTLMASK, al

    push dword[irq_table+4*3*%1+4*2] ;struct interrupt.dev_id
    push %1
	call [irq_table+4*3*%1+4] ;struct interrupt.handler
	add	esp, 4*2

    in al, INT_S_CTLMASK ;恢复当前中断
    and al, ~(1<<(%1-8))
    out INT_S_CTLMASK, al

	ret
%endmacro

hwint00:		; Interrupt routine for irq 1 (keyboard)
	hwint_master 0
hwint01:		; Interrupt routine for irq 1 (keyboard)
	hwint_master 1

; ALIGN	16
hwint02:		; Interrupt routine for irq 2 (cascade!)
   hwint_master 2

; ALIGN	16
hwint03:		; Interrupt routine for irq 3 (second serial)
	hwint_master 3

; ALIGN	16
hwint04:		; Interrupt routine for irq 4 (first serial)
	hwint_master 4

; ALIGN	16
hwint05:		; Interrupt routine for irq 5 (XT winchester)
	hwint_master 5

; ALIGN	16
hwint06:		; Interrupt routine for irq 6 (floppy)
	hwint_master 6

; ALIGN	16
hwint07:		; Interrupt routine for irq 7 (printer)
	hwint_master 7
hwint08:		; Interrupt routine for irq 8 (realtime clock).
	hwint_slave	8

; ALIGN	16
hwint09:		; Interrupt routine for irq 9 (irq 2 redirected)
	hwint_slave	9

; ALIGN	16
hwint10:		; Interrupt routine for irq 10
	hwint_slave	10

; ALIGN	16
hwint11:		; Interrupt routine for irq 11
	hwint_slave	11

; ALIGN	16
hwint12:		; Interrupt routine for irq 12
	hwint_slave	12

; ALIGN	16
hwint13:		; Interrupt routine for irq 13 (FPU exception)
	hwint_slave	13

; ALIGN	16
hwint14:		; Interrupt routine for irq 14 (AT winchester)
	hwint_slave	14

; ALIGN	16
hwint15:		; Interrupt routine for irq 15
	hwint_slave	15