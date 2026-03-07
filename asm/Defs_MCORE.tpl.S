.syntax unified
	.text
	.align	2

/******************************************************************************
 * mcore_veneers.S — Modern M-CORE (RCE) veneers for GCC 15+
 * Single-mode architecture (no Thumb/ARM switching)
 * Works on all Motorola M-CORE phones (big-endian or little-endian)
 * Veneer pattern: lrw r1, addr + jmp r1 (r1 = scratch, r15 = link register)
 ******************************************************************************/

	.macro	def_F	name, addr
	.global	\name
	.type	\name, @function
\name:
		lrw	r1, \addr		@ r1 is volatile scratch (ABI)
		jmp	r1			@ r15 (link register) unchanged → correct return
	.size	\name, . - \name
	.endm

	.macro	def_D	name, addr
	.global	\name
	.set	\name, \addr
	.endm

	.macro	def_S	name, string
	.global	\name
	.section .rodata.str1.4,"aMS",%progbits,1
	.align	2
\name:
		.asciz	"\string"
	.size	\name, . - \name
	.previous
	.endm


	/* ==================== Your symbols (copy-paste as many as you want) ==================== */

%ENTITY_LISTING%

	.end
