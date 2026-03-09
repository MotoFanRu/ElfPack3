	.section .text
	.balign 4

	/*
	 * Functions can simply use constant addresses without veneers on M-CORE, generated code:
	 *   .text:108A8F24 7F 24                       jsri     {off_108A8FB4}
	 *   .text:108A8F26 7F 24                       jsri     {off_108A8FB8}
	 */
	.macro def_M name, addr
	.global \name
	.type \name, %function
	.set \name, \addr
	.endm

	.macro def_D name, addr
	.global \name
	.set \name, \addr
	.endm

	.macro def_S name, string
	.global \name
	.section .rodata.str1.4,"aMS",%progbits,1
	.balign 4
\name:
	.asciz "\string"
	.size \name, . - \name
	.previous
	.endm

%ENTITY_LISTING%
	.end
