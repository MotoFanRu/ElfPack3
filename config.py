from aleph import *

RECIPES: dict[str, Recipe] = {

	# Only for debugging. Delete it later.
	'A830_TALON_U_61.06.00I' : Recipe(
		soc=SOC_RAINBOW, toolchain=P2K_SDK_GCC_MCORE,
		tasks=True,
		flags=Recipe.RecipeFlags(
			build=['-DFTR_A830', '-DFTR_DEBUG'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
		),
		addresses=Recipe.RecipeAddresses(
			register=0x00000000, inject=0x10900000,
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
			register=0x00000000, inject=0x00570000,
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
			register=0x00000000, inject=0x108A8F1C,
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
			register=0x00000000, inject=0x00000000,
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
			register=0x00000000, inject=0x00000000,
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
			register=0x00000000, inject=0x108A8F1C, # TODO: Fix this.
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
			register=0x00000000, inject=0x00000000,
		),
	),
}
