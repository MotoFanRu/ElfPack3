/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola phones on the Patriot SoC, including a memory map.
 *   Supported models: Motorola V60, V60i, V66, V66i, T280, T280i, V70, T720, T720i, T722i, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM        (rx ) : ORIGIN = 0x00000000, LENGTH = 0x00010000 /*   64 KiB */
	IRAM        (rwx) : ORIGIN = 0x00100000, LENGTH = 0x00006800 /*   26 KiB */

	ROM         (rx ) : ORIGIN = 0x10000000, LENGTH = 0x00800000 /*    8 MiB */ /* 4, 8 MiB */
	RAM         (rwx) : ORIGIN = 0x11000000, LENGTH = 0x00200000 /*    2 MiB */ /* 256, 512, 768 KiB, 1, 2 MiB */
}

/* Generate Big-Endian M-CORE ELFs. */
OUTPUT_FORMAT("elf32-mcore-big")

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
