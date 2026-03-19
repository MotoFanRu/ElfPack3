/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola P2K phones on the Neptune LTE SoC, including a memory map.
 *   Supported models: Motorola C380, C650, E398, ROKR E1, RAZR V3, V600, V635, SLVR L6, V80, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM  (rx ) : ORIGIN = 0x00000000, LENGTH = 0x001C0000 /* 1792 KiB */
	IRAM  (rwx) : ORIGIN = 0x03FC0000, LENGTH = 0x00040000 /*  256 KiB */

	ROM   (rx ) : ORIGIN = 0x10000000, LENGTH = 0x02000000 /*   32 MiB */ /* 4, 8, 16, 32 MiB */
	RAM   (rwx) : ORIGIN = 0x12000000, LENGTH = 0x00800000 /*    8 MiB */ /* 2, 4, 8 MiB */
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
INCLUDE "Common_Sections.lds"
