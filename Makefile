NASM = nasm
QEMU = qemu-system-x86_64

BUILD     = build
BOOT_BIN  = $(BUILD)/bootloader/boot.bin
IMG       = $(BUILD)/os.img

.PHONY: all run clean

all: $(IMG)

$(BUILD)/bootloader:
	mkdir -p $@

$(BOOT_BIN): bootloader/boot.asm | $(BUILD)/bootloader
	$(NASM) -f bin $< -o $@

$(IMG): $(BOOT_BIN)
	cp $(BOOT_BIN) $@
	dd if=/dev/zero bs=512 count=2879 >> $@ 2>/dev/null

run: $(IMG)
	$(QEMU) -drive format=raw,file=$(IMG),if=floppy -boot a

clean:
	rm -rf $(BUILD)
