/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola and Siemens P2K phones on the Rainbow PoG SoC, including a memory map.
 *   Supported models: Siemens U15, Motorola A835, C975, V975, E1000, E770, RAZR V3x, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM        (rx ) : ORIGIN = 0x00000000, LENGTH = 0x00006000 /*   24 KiB */
	IRAM_SRAM   (rwx) : ORIGIN = 0x00100000, LENGTH = 0x00000800 /*    2 KiB */
	IRAM_EDRAM  (rwx) : ORIGIN = 0x07800000, LENGTH = 0x00014000 /*   80 KiB */

	ROM         (rx ) : ORIGIN = 0x10000000, LENGTH = 0x04000000 /*   64 MiB */ /* 8, 16, 32, 64 MiB */
	RAM         (rwx) : ORIGIN = 0x08000000, LENGTH = 0x02000000 /*   32 MiB */ /* 2, 4, 8, 16, 32 MiB */
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
INCLUDE "Common_Sections.lds"
