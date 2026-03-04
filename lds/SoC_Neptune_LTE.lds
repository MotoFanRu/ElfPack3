/*
 * Project:
 *   EP3: ElfPack 3.x by MotoFan.Ru for Motorola P2K phones.
 *
 * About:
 *   Linker script for Motorola P2K phones on the Neptune LTE SoC, including a memory map.
 *   The PATCH/ORIGIN start address and entry point function must be replaced with a valid one.
 *   Supported models: Motorola E398, ROKR E1, RAZR V3, C650, C380 etc.
 *
 * Author:
 *   EXL, 04-Mar-2026
 */

MEMORY {
	IROM  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x001C0000 /* 1.75  MiB     */
	IRAM  (rwx) : ORIGIN = 0x03FC0000, LENGTH = 0x00040000 /* 256   KiB     */
	ROM   (rx)  : ORIGIN = 0x10000000, LENGTH = 0x02000000 /* 16-32 MiB     */
	RAM   (rwx) : ORIGIN = 0x12000000, LENGTH = 0x00800000 /* 4-8   MiB     */

	/* Entry point origin address. */
	PATCH (rx)  : ORIGIN = %addr_entry%, LENGTH = ORIGIN(ROM) + LENGTH(ROM) - ORIGIN(PATCH)
}

/* Generate Big-Endian ARM ELFs. */
OUTPUT_FORMAT("elf32-bigarm")

/* Entry point function. */
ENTRY(%entry_point_function%)

/* Combine all relevant section to one. */

SECTIONS {
	.text : {
		KEEP(*(.text.entry_point))
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(4);
	} > PATCH
}
