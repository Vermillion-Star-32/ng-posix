NASM  = nasm
CC    = i686-elf-gcc
LD    = i686-elf-ld
QEMU  = qemu-system-x86_64

CFLAGS  = -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -m32 -O0 -g
LDFLAGS = --oformat binary -T linker.ld

OBJ = hal/vga.o kernel/kernel.o

.PHONY: all run clean

all: os.img

bootloader/boot.bin: bootloader/boot.asm
	$(NASM) -f bin $< -o $@

hal/%.o: hal/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.bin: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

os.img: bootloader/boot.bin kernel.bin
	cat bootloader/boot.bin kernel.bin > $@
	truncate -s %512 $@

run: os.img
	$(QEMU) -drive format=raw,file=os.img -display curses

clean:
	rm -f bootloader/boot.bin hal/*.o kernel/*.o kernel.bin os.img
