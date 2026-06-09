ENTRY(_start)

SECTIONS {
    . = 0x8000;

    .text : {
        *(.entry)
        *(.text)
    }

    .rodata : { *(.rodata) }

    .data : { *(.data) }

    .bss : {
        bss_start = .;
        *(.bss)
        *(COMMON)
        bss_end = .;
    }
}
