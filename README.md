# ng-posix

A barebones x86 OS built from scratch in assembly and C.

---

## Project structure

```
ng-posix/
├── Makefile
├── linker.ld
├── bootloader/
│   └── boot.asm
├── hal/
│   ├── vga.c
│   └── isr_stub.asm
└── kernel/
    ├── entry.asm
    ├── kernel.h
    ├── kernel.c
    ├── idt.h
    ├── idt.c
    └── isr.c
```

---

## Requirements

| Tool | Purpose | Install |
|---|---|---|
| `nasm` | Assembles `.asm` files | `apt install nasm` |
| `i686-elf-gcc` | Cross-compiles C for bare-metal x86 | [OSDev guide](https://wiki.osdev.org/GCC_Cross-Compiler) |
| `i686-elf-ld` | Links object files into flat binary | Comes with cross-compiler |
| `qemu-system-x86_64` | Runs the OS image | `apt install qemu-system-x86` |

---

## Build

```bash
make          # build build/os.img
make run      # build and launch in QEMU
make clean    # remove build/ entirely
```
---

## Memory layout

| Address | Contents |
|---|---|
| `0x0500` – `0x6FFF` | Free — available for use |
| `0x7000` | Stack base (grows downward) |
| `0x7C00` – `0x7DFF` | Bootloader (512 bytes, loaded by BIOS) |
| `0x7E00` – `0x7FFF` | Free |
| `0x8000` | Kernel load target — `_start` lands here |
| `0x8FFE` – `0x8FFF` | E820 memory map entry count (one word) |
| `0x9000` – `0x9FFF` | E820 memory map buffer |
| `0x90000` | Kernel stack top (set in protected mode) |

---

## Boot sequence

```
BIOS
 └─ loads boot sector (512 bytes) at 0x7C00, jumps to it

bootloader/boot.asm
 ├─ 1. CPU setup     — disable interrupts, save boot drive (DL),
 │                     zero segment registers, move stack to 0x7000,
 │                     set direction flag (cld), far jump to flush CS
 ├─ 2. detect_memory — INT 0x15 E820, builds memory map at 0x9000,
 │                     entry count stored at 0x8FFE
 ├─ 3. enable_a20    — BIOS INT 0x15 AX=0x2401 first,
 │                     keyboard controller (KBC) fallback
 ├─ 4. load_kernel   — LBA (INT 0x13 AH=0x42) preferred,
 │                     CHS (INT 0x13 AH=0x02) fallback,
 │                     loads 32 sectors into 0x8000
 └─ 5. enter_pm      — loads GDT (null + code + data descriptors),
                        sets CR0.PE bit, far jumps to 32-bit code,
                        sets up segment registers + stack at 0x90000,
                        jumps to 0x8000

kernel/entry.asm  (_start at 0x8000)
 ├─ zeroes BSS segment (bss_start..bss_end from linker script)
 └─ calls kernel_main()

kernel/kernel.c
 ├─ vga_clear()
 ├─ prints banner
 ├─ idt_init()
 └─ hlt loop
```

---

## Components

### bootloader/boot.asm

**Key routines:**

- `detect_memory` — iterates E820 entries via BIOS INT 0x15 AX=0xE820.
  Each entry is 24 bytes: base address (8), length (8), type (4), ACPI flags (4).
  Type 1 = usable RAM. Entry count written to `0x8FFE` for the kernel to read.

- `enable_a20` — the A20 line gates access to memory above 1MB. Without it,
  any address with bit 20 set wraps around to 0. Tries the BIOS method first
  (one instruction), falls back to toggling the keyboard controller output port.

- `load_kernel` — reads `KERNEL_SECTORS` (32) sectors from the boot drive
  starting at LBA sector 1 (sector 2 in 1-indexed CHS). LBA mode is tried
  first via INT 0x13 AH=0x42 with a Disk Address Packet; if the BIOS reports
  no LBA support, falls back to CHS geometry with INT 0x13 AH=0x02.
  Halts with an error message on failure.

- `enter_pm` — installs a minimal GDT (null, ring-0 code, ring-0 data),
  sets the PE bit in CR0, and does a far jump to flush the prefetch queue
  and reload CS with the code segment descriptor (0x08).

**GDT layout:**

| Index | Offset | Descriptor |
|---|---|---|
| 0 | 0x00 | Null (required) |
| 1 | 0x08 | Code — base 0, limit 4GB, ring 0, executable |
| 2 | 0x10 | Data — base 0, limit 4GB, ring 0, writable |

---

### kernel/entry.asm

First code that runs at `0x8000` after the bootloader jumps. Runs in 32-bit
protected mode. Uses section `.entry` so the linker script can guarantee it
is placed before all other `.text` sections.

- Zeroes the BSS region using `rep stosb` between `bss_start` and `bss_end`
  (symbols emitted by `linker.ld`). This ensures C globals and statics are
  zero-initialised as the C standard requires, since bare-metal has no OS
  loader to do this.
- Calls `kernel_main()`. If `kernel_main` ever returns (it shouldn't),
  disables interrupts and halts.

---

### kernel/kernel.c

Entry point for C code. Called by `entry.asm`.

Current responsibilities:
- Clear the VGA screen
- Print the OS banner
- Initialise the IDT (`idt_init`)
- Halt in a loop

---

### hal/vga.c

VGA text mode driver. Writes directly to the memory-mapped VGA buffer.

| Detail | Value |
|---|---|
| Buffer address | `0xB8000` |
| Dimensions | 80 columns × 25 rows |
| Cell format | 2 bytes: `[char][color]` |
| Default color | `0x0F` — white on black |

**Functions:**

- `vga_clear()` — fills all 2000 cells with spaces, resets cursor to 0
- `vga_putchar(c)` — writes one character; `
` advances cursor to next row
- `vga_print(str)` — writes a null-terminated string
- `vga_print_hex(n)` — writes an unsigned int as `0xNNNN`

---

### kernel/idt.h + kernel/idt.c

Interrupt Descriptor Table — tells the CPU where to jump for each interrupt.

The IDT has 256 entries. Each entry (`idt_entry_t`) is 8 bytes:

| Field | Bits | Description |
|---|---|---|
| `base_low` | 0:15 | Low 16 bits of handler address |
| `selector` | 16:31 | Code segment selector (0x08) |
| `zero` | 32:39 | Reserved, must be 0 |
| `flags` | 40:47 | `0x8E` = present, ring 0, 32-bit interrupt gate |
| `base_high` | 48:63 | High 16 bits of handler address |

`idt_init()` installs handlers for all 32 CPU exceptions (ISRs 0–31) and
loads the IDT register with `lidt`. IRQ handlers (ISRs 32–47) will be
installed when the PIC is remapped.

`registers_t` matches the exact stack layout pushed by `isr_stub.asm`:

```
High address
  ss, useresp, eflags, cs, eip   ← CPU pushes automatically
  err_code, int_no                ← stub pushes
  eax, ecx, edx, ebx, esp, ebp, esi, edi  ← pusha
  ds, es, fs, gs                  ← stub pushes segments
Low address  ← ESP points here (registers_t*)
```

---

### hal/isr_stub.asm

32 ISR entry stubs generated with NASM macros.

Two macro variants:
- `ISR_NOERRCODE n` — pushes dummy `0` error code then interrupt number
- `ISR_ERRCODE n` — CPU already pushed error code, just pushes interrupt number

Exceptions that push an error code: 8, 10, 11, 12, 13, 14, 17, 30.

`isr_common` (shared tail):
1. `pusha` — saves all general purpose registers
2. Push segment registers (`ds, es, fs, gs`)
3. Load kernel data segment into all segment registers
4. Push `esp` as `registers_t*` argument
5. Call `isr_handler`
6. Restore segments, `popa`, discard `int_no`+`err_code`, `iret`

---

### kernel/isr.c

C-level exception handler called from every ISR stub.

`isr_handler(registers_t *regs)` prints:
- Exception name (looked up from a table of all 32 CPU exceptions)
- ISR number in hex
- Faulting `EIP` — address of the instruction that caused the exception
- Error code — for exceptions that provide one (page fault, GPF etc.)

Then disables interrupts and halts permanently.

---

## Linker script (linker.ld)

Places the kernel at `0x8000`. Section order:

```
0x8000   .text    — .entry first (entry.asm), then all other code
         .rodata  — read-only data (string literals etc.)
         .data    — initialised globals
         .bss     — zero-initialised globals (bss_start / bss_end exported)
```

---

## What's next

| Step | Description |
|---|---|
| PIC remapping | Remap 8259 PIC so hardware IRQs land at ISR 32–47 |
| Keyboard driver | `hal/keyboard.c` — IRQ1 handler, scancode translation |
| Physical memory manager | Page allocator using the E820 map at `0x9000` |
| Heap | `kmalloc` / `kfree` on top of the page allocator |
| Processes | Task structs, context switching, basic scheduler |
