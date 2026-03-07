/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola P2K phones on the Neptune LT (LCA) SoC, including a memory map.
 *   Supported models: Motorola C330, C350, C450, C550, E380, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM        (rx ) : ORIGIN = 0x10000000, LENGTH = 0x00318000 /* 3168 KiB */
	IRAM_MXRAM  (rwx) : ORIGIN = 0x01400000, LENGTH = 0x00020000 /*  128 KiB */
	IRAM_SRAM   (rwx) : ORIGIN = 0x01F93000, LENGTH = 0x0006D000 /*  436 KiB */

	ROM         (rx ) : ORIGIN = 0x00000000, LENGTH = 0x01000000 /*   16 MiB */ /* 4, 8, 16 MiB */
	RAM         (rwx) : ORIGIN = 0x11000000, LENGTH = 0x00400000 /*    4 MiB */ /* 1, 2, 4 MiB */
}

/* Generate Big-Endian ARM ELFs. */
OUTPUT_FORMAT("elf32-bigarm")

/*
 * Entry point symbol.
 * Can be set using GCC command-line arguments:
 *   -Wl,-e,%ENTRY_POINT%
 *
ENTRY(%ENTRY_POINT%)
 */

/* Combine all relevant code and data section to the one `.text` section. */
SECTIONS {
	/*
	 * Entry point address.
	 * Can be set using GCC command-line arguments in 0x10000000 HEX format:
	 *   -Wl,-e,%ENTRY_ADDRESS%
	 *
	. = %ENTRY_ADDRESS%;
	 */

	.text : {
		KEEP(*(.text.entry_point))
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(4);
	}
}
