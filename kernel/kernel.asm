global DispStr
global in_byte
global out_byte

global _start

extern main

PAGE_OFFSET equ 0xC0000000
STACKTOP equ 0x7e00

[SECTION .data]
disp_pos dd	0

[SECTION .text]
[BITS 32]

_start:
    mov esp, STACKTOP+PAGE_OFFSET
    call main
    jmp $

in_byte:
    mov edx, [esp+4]
    xor eax, eax
    in al, dx
    nop
    nop
    ret

out_byte:
    mov edx, [esp+4]
    mov al, [esp+4+4]
    out dx, al
    nop
    nop
    ret

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
