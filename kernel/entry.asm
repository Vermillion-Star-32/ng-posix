[BITS 32]

section .entry
global _start
extern kernel_main

_start:
    ; Clear direction flag
    cld

    ; Zero BSS section so C global/static vars start at 0
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Call into C kernel
    call kernel_main

    ; kernel_main should never return — halt if it does
    cli
.hang:
    hlt
    jmp .hang

extern bss_start
extern bss_end
