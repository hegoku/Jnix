PAGE_DIR_BASE equ 0x1000
PAGE_TBL_BASE equ 0x1000+1000H

[BITS 32]
EnablePaging:
    pushad

    mov dword[PAGE_DIR_BASE], PAGE_TBL_BASE | PG_P | PG_USS | PG_RWW
    mov dword[PAGE_DIR_BASE+4], PAGE_TBL_BASE+1000H | PG_P | PG_USS | PG_RWW ;第二个4MB
    mov dword[PAGE_DIR_BASE+3072], PAGE_TBL_BASE | PG_P | PG_USS | PG_RWW
    mov dword[PAGE_DIR_BASE+3072+4], PAGE_TBL_BASE+1000H | PG_P | PG_USS | PG_RWW
    mov edi, PAGE_TBL_BASE+4092+4096
    ; mov eax, 03ff007H  ;4Mb - 4096 + 7 (r/w user,p)
    mov eax, 07ff003H  ;8Mb - 4096 + 7 (r/w supervisor,p)
    mov ecx, 1024*2
    std
 .1:
    stosd
    sub eax, 0x1000
    loop .1
    cld

    mov eax, PAGE_DIR_BASE
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    popad
    ret