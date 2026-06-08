; boot.asm - Minimal bootloader
; Prints "Hello from bootloader!" using BIOS interrupt and halts.

[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, msg
.loop:
    lodsb
    test al, al
    jz .hang
    mov ah, 0x0E
    int 0x10
    jmp .loop

.hang:
    cli
    hlt
    jmp .hang

msg db "Hello from bootloader!", 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55
