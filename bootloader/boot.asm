[BITS 16]
[ORG 0x7C00]

KERNEL_OFFSET  equ 0x8000
KERNEL_SECTORS equ 32
STACK_BASE     equ 0x7000
E820_BUFFER    equ 0x9000
CODE_SEG       equ 0x08
DATA_SEG       equ 0x10

; =============================================================================
; Entry
; =============================================================================
start:
    cli
    mov [boot_drive], dl    ; preserve BIOS boot drive
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, STACK_BASE
    cld                     ; forward direction for string ops
    jmp 0x0000:main         ; far jump — flush CS

main:
    sti
    call detect_memory
    call enable_a20
    call load_kernel
    call enter_pm
    cli
    hlt

; =============================================================================
; print16 — print null-terminated string at SI
; =============================================================================
print16:
    pusha
.l: lodsb
    test al, al
    jz .d
    mov ah, 0x0E
    int 0x10
    jmp .l
.d: popa
    ret

; =============================================================================
; detect_memory — E820 map into E820_BUFFER, count at E820_BUFFER-2
; =============================================================================
detect_memory:
    pusha
    mov di, E820_BUFFER
    xor ebx, ebx
    xor bp, bp
    mov edx, 0x534D4150
.next:
    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    jc .done
    cmp eax, 0x534D4150
    jne .done
    test ecx, ecx
    jz .skip
    inc bp
    add di, 24
.skip:
    test ebx, ebx
    jz .done
    jmp .next
.done:
    mov [E820_BUFFER - 2], bp
    popa
    ret

; =============================================================================
; enable_a20 — BIOS method first, KBC fallback
; =============================================================================
enable_a20:
    pusha
    mov ax, 0x2401
    int 0x15
    jnc .done
    ; KBC fallback
    call .kbc_in
    mov al, 0xAD
    out 0x64, al
    call .kbc_in
    mov al, 0xD0
    out 0x64, al
    call .kbc_out
    in  al, 0x60
    push ax
    call .kbc_in
    mov al, 0xD1
    out 0x64, al
    call .kbc_in
    pop ax
    or  al, 2
    out 0x60, al
    call .kbc_in
    mov al, 0xAE
    out 0x64, al
.done:
    popa
    ret
.kbc_in:
    in  al, 0x64
    test al, 2
    jnz .kbc_in
    ret
.kbc_out:
    in  al, 0x64
    test al, 1
    jz  .kbc_out
    ret

; =============================================================================
; load_kernel — LBA preferred, CHS fallback
; =============================================================================
load_kernel:
    pusha
    ; Check LBA support
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc  .chs
    cmp bx, 0xAA55
    jne .chs
.lba:
    mov byte [dap],      16
    mov byte [dap + 1],  0
    mov word [dap + 2],  KERNEL_SECTORS
    mov word [dap + 4],  KERNEL_OFFSET
    mov word [dap + 6],  0x0000
    mov dword [dap + 8], 1
    mov dword [dap + 12], 0
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc  .err
    jmp .ok
.chs:
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, KERNEL_OFFSET
    int 0x13
    jc  .err
.ok:
    popa
    ret
.err:
    mov si, msg_err
    call print16
    cli
    hlt

; =============================================================================
; enter_pm — load GDT, set PE, far jump to 32-bit
; =============================================================================
enter_pm:
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp CODE_SEG:pm32

[BITS 32]
pm32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x90000
    mov esp, ebp
    hlt ;  temp
    jmp KERNEL_OFFSET

; =============================================================================
; GDT
; =============================================================================
gdt_start:
    dq 0
gdt_code: dw 0xFFFF, 0x0000
          db 0x00, 10011010b, 11001111b, 0x00
gdt_data: dw 0xFFFF, 0x0000
          db 0x00, 10010010b, 11001111b, 0x00
gdt_end:
gdt_desc: dw gdt_end - gdt_start - 1
          dd gdt_start

; =============================================================================
; Data
; =============================================================================
dap:        times 16 db 0
boot_drive: db 0
msg_err:    db "Disk error!", 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55
