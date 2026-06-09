NASM = nasm
CC   = i686-elf-gcc
LD   = i686-elf-ld
QEMU = qemu-system-x86_64

BUILD   = build
IMG     = $(BUILD)/os.img

CFLAGS  = -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -m32 -O0 -g
LDFLAGS = --oformat binary -T linker.ld -Map $(BUILD)/kernel.map

OBJ = $(BUILD)/kernel/entry.o \
      $(BUILD)/hal/vga.o       \
      $(BUILD)/kernel/kernel.o

.PHONY: all run clean

all: $(IMG)

$(BUILD)/bootloader $(BUILD)/hal $(BUILD)/kernel:
	mkdir -p $@

$(BUILD)/bootloader/boot.bin: bootloader/boot.asm | $(BUILD)/bootloader
	$(NASM) -f bin $< -o $@

$(BUILD)/kernel/entry.o: kernel/entry.asm | $(BUILD)/kernel
	$(NASM) -f elf32 $< -o $@

$(BUILD)/hal/%.o: hal/%.c | $(BUILD)/hal
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel/%.o: kernel/%.c | $(BUILD)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.bin: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

$(IMG): $(BUILD)/bootloader/boot.bin $(BUILD)/kernel.bin
	cat $^ > $@
	dd if=/dev/zero bs=512 count=2880 of=$(IMG) conv=notrunc 2>/dev/null
	dd if=$(BUILD)/bootloader/boot.bin of=$(IMG) conv=notrunc bs=512 seek=0 2>/dev/null
	dd if=$(BUILD)/kernel.bin of=$(IMG) conv=notrunc bs=512 seek=1 2>/dev/null

run: $(IMG)
	$(QEMU) -drive format=raw,file=$(IMG),if=floppy -boot a

clean:
	rm -rf $(BUILD)
