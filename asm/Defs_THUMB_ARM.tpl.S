.syntax unified                                            @ Unified Assembly Language (UAL) syntax.
	.text                                                  @ Use `.text` section in object file.
	.balign 4                                              @ Use 4-byte alignment (ARM instructions + `.word` data).
	.thumb                                                 @ Force Thumb mode for the whole listing.

	.macro def_T name, addr                                @ Thumb function veneer macro (sets LSB=1).
	.global \name                                          @ Export symbol so linker can see it.
	.type \name, %function                                 @ Mark it as function (enables interworking + debug info).
	.thumb                                                 @ Force Thumb mode for function.
	.thumb_func                                            @ Specifies that symbol is the name of a Thumb function.
	.balign 4                                              @ Force 4-byte alignment.
\name:                                                     @ Veneer entry point (called from Thumb or ARM code).
		bx pc                                              @ Immediate clean mode switch.
		nop                                                @ 2-byte padding for word alignment + pipeline.
	.arm                                                   @ Ensure this veneer is assembled in ARM mode.
		ldr r12, .Ltarget_\name                            @ Load from label.
		bx r12                                             @ Branch-and-exchange, switches mode automatically if Thumb.
.Ltarget_\name:                                            @ Label with the Thumb address target.
		.word (\addr | 1)                                  @ Target address with Thumb bit set (LSB=1).
	.size \name, . - \name                                 @ Tell linker/debugger the exact size of this veneer.
	.endm                                                  @ End Thumb function veneer macro.

	.macro def_A name, addr                                @ ARM function veneer macro (sets LSB=0).
	.global \name
	.type \name, %function
	.thumb
	.thumb_func
	.balign 4
\name:
		bx pc
		nop
	.arm
		ldr r12, .Ltarget_\name
		bx r12
.Ltarget_\name:                                            @ Label with the ARM address target.
		.word (\addr | 0)                                  @ Pure ARM target address with no bit set (LSB=0).
	.size \name, . - \name
	.endm                                                  @ End ARM function veneer macro.

	.macro def_D name, addr                                @ Data global address macro (no code generated).
	.global \name                                          @ Just an absolute symbol for linker.
	.set \name, \addr
	.endm                                                  @ End Data global address macro.

	.macro def_S name, string                              @ Null-terminated ASCII string macro (in `.rodata` section).
	.global \name
	.section .rodata.str1.4,"aMS",%progbits,1              @ Read-only string section (mergeable, 4-byte aligned).
	.balign 4                                              @ Use 4-byte alignment.
\name:
		.asciz "\string"                                   @ Insert zero-terminated string.
	.size \name, . - \name
	.previous                                              @ Return to previous `.text` section.
	.endm                                                  @ End Null-terminated ASCII string macro.

%ENTITY_LISTING%

	.end                                                   @ End of assembly listing file.
