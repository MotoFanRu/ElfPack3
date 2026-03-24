from aleph import *

RECIPES: dict[str, Recipe] = {

	# Only for debugging. Delete it later.
	'A830_TALON_U_61.06.00I' : Recipe(
		soc=SOC_RAINBOW, toolchain=P2K_SDK_GCC_MCORE,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_A830', '-DFTR_DEBUG'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI', '-DFTR_NO_VSNPRINTF'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			inject=0x10900000, loader=0x07AA0000 + 0x20, register=0x106CF134, task_a=0x1053B4E4, task_b=0x1053BB44,
		),
	),
	'C330_TA02_G_06.03.25R_elf' : Recipe(
		soc=SOC_NEPTUNE_LT_LCA, toolchain=P2K_SDK_GCC_ARM,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_C330', '-DFTR_DEBUG'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_NULL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_NULL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			inject=0x00570000, loader=0x1106B010 + 0x20, register=0x004307FC, task_a=0x00078F78, task_b=0x0007896C,
		),
	),
	'E1_R373_G_0E.30.44R_elf' : Recipe(
		soc=SOC_NEPTUNE_LTE, toolchain=P2K_SDK_GCC_ARM,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_E1', '-DFTR_DEBUG'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			inject=0x108A8F1C, loader=0x1279A540 + 0x20, register=0x103F977C, task_a=0x10A5D494, task_b=0x10A5E5EC,
		),
	),

	'E1_R373_G_0E.30.49R' : Recipe(
		soc=SOC_NEPTUNE_LTE, toolchain=P2K_SDK_GCC_ARM,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_E1'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			inject=0x00000000, loader=0x00000000, register=0x00000000, task_a=0x00000000, task_b=0x00000000,
		),
	),

	'V3i_R4441D_G_08.01.03R' : Recipe(
		soc=SOC_NEPTUNE_LTE2, toolchain=P2K_SDK_GCC_ARM,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_V3I'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			inject=0x00000000, loader=0x00000000, register=0x00000000, task_a=0x00000000, task_b=0x00000000,
		),
	),

	'V3x_R252211LD_U_85.9B.E6P' : Recipe(
		soc=SOC_RAINBOW_POG, toolchain=P2K_SDK_GCC_MCORE,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_V3X'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			# TODO: Fix this.
			inject=0x108A8F1C, loader=0x00000000, register=0x00000000, task_a=0x00000000, task_b=0x00000000,
		),
	),

	'V3xx_R26111LD_U_96.A0.0ER' : Recipe(
		soc=SOC_ARGON_LV, toolchain=P2K_SDK_GCC_ARM,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_V3XX'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			inject=0x00000000, loader=0x00000000, register=0x00000000, task_a=0x00000000, task_b=0x00000000,
		),
	),
}
