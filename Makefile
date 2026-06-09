NASM 				= nasm
CC   				= i686-elf-gcc
LD   				= i686-elf-ld
QEMU 				= qemu-system-x86_64
BUILD   			= build
IMG     			= $(BUILD)/os.img
KERNEL_C 			= $(wildcard kernel/*.c)
HAL_C    			= $(wildcard hal/*.c)
KERNEL_ASM 			= kernel/entry.asm
HAL_ASM    			= hal/isr_stub.asm

CFLAGS  = -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -m32 -O0 -g -Iinclude
LDFLAGS = --oformat binary -T linker.ld -Map $(BUILD)/kernel.map

OBJ = $(BUILD)/kernel/entry.o $(HAL_C:hal/%.c=$(BUILD)/hal/%.o) $(KERNEL_C:kernel/%.c=$(BUILD)/kernel/%.o) $(BUILD)/hal/isr_stub.o

.PHONY: all run clean dirs

# ====== Makefile commands ======

all: $(IMG)

run: $(IMG)
	$(QEMU) -drive format=raw,file=$(IMG),if=floppy -boot a

clean:
	rm -rf $(BUILD)

dirs:
	mkdir -p $(BUILD)/bootloader $(BUILD)/hal $(BUILD)/kernel

# ====== Compiling scripts ======

# ASM files
$(BUILD)/bootloader/boot.bin: bootloader/boot.asm | dirs
	$(NASM) -f bin $< -o $@

$(BUILD)/kernel/entry.o: kernel/entry.asm | dirs
	$(NASM) -f elf32 $< -o $@

$(BUILD)/hal/isr_stub.o: hal/isr_stub.asm | dirs
	$(NASM) -f elf32 $< -o $@

$(BUILD)/kernel/%.o: kernel/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# GNU Linker Script
$(BUILD)/kernel.bin: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

# C files
$(BUILD)/hal/%.o: hal/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# Image creation
$(IMG): $(BUILD)/bootloader/boot.bin $(BUILD)/kernel.bin
	dd if=/dev/zero bs=512 count=2880 of=$(IMG) 2>/dev/null
	dd if=$(BUILD)/bootloader/boot.bin of=$(IMG) conv=notrunc bs=512 seek=0 2>/dev/null
	dd if=$(BUILD)/kernel.bin of=$(IMG) conv=notrunc bs=512 seek=1 2>/dev/null