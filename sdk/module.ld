/* Standalone zero-linked module layout. NOT a game installation address. */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(egg_module_entry)
SECTIONS {
    . = 0;
    .text : { KEEP(*(.text.egg_module_entry)) *(.text .text.*) }
    .rodata ALIGN(4) : { *(.rodata .rodata.*) }
    . = ALIGN(4096);
    .data : { *(.data .data.*) }
    .bss ALIGN(4) (NOLOAD) : { *(.bss .bss.*) }
    /DISCARD/ : { *(.comment) *(.note*) }
}
