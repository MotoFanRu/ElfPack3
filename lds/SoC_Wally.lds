/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola phones on the Wally SoC (CDMA), including a memory map.
 *   Supported models: Motorola V120c, V60c, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM        (rx ) : ORIGIN = 0x00000000, LENGTH = 0x00008000 /*   32 KiB */
	IRAM        (rwx) : ORIGIN = 0x00100000, LENGTH = 0x00001000 /*    4 KiB */

	ROM         (rx ) : ORIGIN = 0x40000000, LENGTH = 0x00400000 /*    4 MiB */ /* 2, 4 MiB */
	RAM         (rwx) : ORIGIN = 0x41000000, LENGTH = 0x00100000 /*    1 MiB */ /* 256, 512, 768 KiB, 1 MiB */
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
