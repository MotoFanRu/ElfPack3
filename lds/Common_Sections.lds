/*
 * Combine all relevant code and data section to the one `.text` section.
 * Common for all SoCs and Architectures.
 */
SECTIONS {
	/*
	 * Entry point address.
	 * Can be set using GCC command-line arguments in 0x10000000 HEX format:
	 *   -Wl,-e,%ENTRY_ADDRESS%
	 *
	. = %ENTRY_ADDRESS%;
	 */

	.text : {
		KEEP(*(.text.bin.entry_point))
		KEEP(*(.text.task.entry_point))
		KEEP(*(.text.task.reactor))
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(4);
	}
}
