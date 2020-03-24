ReadMemorySize:
    pushad
    push ds
    push es
    xor eax, eax
    mov ds, ax
    mov es, ax
    mov ebx, 0
    ; mov ax, _MemChkBuf
    ; mov [ds:0x500], ax ;内核会从这个地址取_MemChkbuf的地址
    ; mov ax, _dwMCRNumber
    ; mov [ds:0x502], ax ;内核会从这个地址取_dwMCRNumber的地址
    ; mov di, _MemChkBuf
    mov word[ds:BIOS_ADDR], bx
    mov di, BIOS_ADDR+2
.ReadMemorySizeLoop:
    mov eax, 0E820h
    mov ecx, 20
    mov edx, 0534D4150h
    int 15h
    jc .LABEL_MEM_CHK_FAIL
    add di, 20
    ; inc dword[_dwMCRNumber]
    inc dword[ds:BIOS_ADDR]
    cmp ebx, 0
    jne .ReadMemorySizeLoop
    jmp .LABEL_MEM_CHK_OK
.LABEL_MEM_CHK_FAIL:
    ; mov dword [_dwMCRNumber], 0
    mov dword [ds:BIOS_ADDR], 0
.LABEL_MEM_CHK_OK:
    call CalMemSize
    pop es
    pop ds
    popad
    ret

CalMemSize:
    xor esi, ecx
    xor ecx, ecx
    mov esi, BIOS_ADDR+2
    mov ecx, dword[ds:BIOS_ADDR]
.cloop:
    mov eax, dword[esi+4*4]
    cmp eax, 1
    jne .2
    mov eax, dword[esi+0]
    add eax, dword[esi+2*4]
    cmp eax, dword[mem_size]
    jb .2
    mov dword[mem_size], eax
.2:
    add esi, 5*4
    loop .cloop
    ret

mem_size dd 0