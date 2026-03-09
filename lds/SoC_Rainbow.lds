/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola and Siemens P2K phones on the Rainbow SoC, including a memory map.
 *   Supported models: Siemens U10, Motorola A820, A830, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM        (rx ) : ORIGIN = 0x00000000, LENGTH = 0x00004000 /*   16 KiB */
	IRAM_SRAM   (rwx) : ORIGIN = 0x00100000, LENGTH = 0x00000800 /*    2 KiB */
	IRAM_EDRAM  (rwx) : ORIGIN = 0x07800000, LENGTH = 0x00800000 /*    8 MiB */ /* 3, 8 MiB */

	ROM         (rx ) : ORIGIN = 0x10000000, LENGTH = 0x01000000 /*   16 MiB */ /* 8, 16 MiB */
	/* A820, A830, U10 do not have any external RAM memory chip. */
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
