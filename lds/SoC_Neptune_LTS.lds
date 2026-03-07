/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola P2K phones on the Neptune LTS SoC, including a memory map.
 *   Supported models: Motorola C350 T-Mobile, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM  (rx ) : ORIGIN = 0x00000000, LENGTH = 0x001C0000 /* 1792 KiB */
	IRAM  (rwx) : ORIGIN = 0x03F80000, LENGTH = 0x00080000 /*  512 KiB */

	ROM   (rx ) : ORIGIN = 0x10000000, LENGTH = 0x00800000 /*    8 MiB */
	RAM   (rwx) : ORIGIN = 0x12000000, LENGTH = 0x00200000 /*    2 MiB */
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
