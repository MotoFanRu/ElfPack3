/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola P2K phones on the Argon+ (Plus) SoC, including a memory map.
 *   Supported models: Motorola M702iG, M702iS, etc.
 *
 * Author:
 *   EXL, 05-Mar-2026
 */

MEMORY {
	IROM1  (rx ) : ORIGIN = 0x00000000, LENGTH = 0x00004000 /*   16 KiB */
	IROM2  (rx ) : ORIGIN = 0x00404000, LENGTH = 0x00004000 /*   16 KiB */
	IRAM   (rwx) : ORIGIN = 0x1FFFC000, LENGTH = 0x00004000 /*   16 KiB */

	ROM    (rx ) : ORIGIN = 0xA0000000, LENGTH = 0x04000000 /*   64 MiB */ /* 4, 8, 16, 32, 64 MiB */
	RAM    (rwx) : ORIGIN = 0x80000000, LENGTH = 0x04000000 /*   64 MiB */ /* 8, 16, 32, 64 MiB */
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
