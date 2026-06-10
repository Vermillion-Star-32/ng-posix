; =================================================================================
; entry.asm: This is where it starts after bootloader is done
;  
; Purpose:
;   This file defines the first instruction executed after bootloader hands control
;   to kernel in protected mode
; 
; Execution:
;   1) Clear direction flag, set DF=0
;   2) Zero .bss section
;   3) Call kernal_main (C code)
;   4) Enter infinite hlt loop if kernel.c returns (Should not happen)
; 
; =================================================================================

[BITS 32]

section .entry
global _start
extern kernel_main

_start:
    ; DF might be left in wrong state, make sure .bss clearing is forward with DF=0
    cld

    ; Fill bss region with zeroes (To be deleted once standard runtime implemented)
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; The C code
    call kernel_main

    ; Disable interrupts after kernel exit (To be removed once OS scheduler implemented)
    cli
.hang:
    hlt
    jmp .hang

extern bss_start
extern bss_end