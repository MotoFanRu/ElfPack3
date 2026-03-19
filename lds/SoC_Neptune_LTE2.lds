/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola P2K phones on the Neptune LTE2 ROM3 SoC, including a memory map.
 *   Supported models: Motorola V235, V360, SLVR L7, SLVR L7e, RAZR V3r, RAZR V3i, KRZR K1, RIZR Z3, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM  (rx ) : ORIGIN = 0x00000000, LENGTH = 0x00020000 /*  128 KiB */
	IRAM  (rwx) : ORIGIN = 0x03F80000, LENGTH = 0x00080000 /*  512 KiB */

	ROM1  (rx ) : ORIGIN = 0x10000000, LENGTH = 0x02000000 /*   32 MiB */ /* 4, 8, 16, 32 MiB */
	RAM1  (rwx) : ORIGIN = 0x12000000, LENGTH = 0x00800000 /*    8 MiB */ /* 2, 4, 8 MiB */

	ROM2  (rx ) : ORIGIN = 0x10000000, LENGTH = 0x04000000 /*   64 MiB */ /* 48, 64 MiB */
	RAM2  (rwx) : ORIGIN = 0x14000000, LENGTH = 0x01000000 /*   16 MiB */
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
