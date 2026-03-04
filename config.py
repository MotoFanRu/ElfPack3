from aleph import Recipe

from aleph import P2K_SDK_GCC_ARM
from aleph import P2K_SDK_GCC_MCORE

RECIPES: dict[str, Recipe] = {

	'E1_R373_G_0E.30.49R' : Recipe(
		toolchain=P2K_SDK_GCC_ARM,
		flags=Recipe.RecipeFlags(
			cpu=['-mbig-endian', '-mthumb', '-mthumb-interwork', '-march=armv4t', '-mtune=arm7tdmi-s'],
			cpu_flavor=['-DFTR_ARM', '-DFTR_ARM7', '-DFTR_NEPTUNE', '-DFTR_NEPTUNE_LTE'],
			build=['-D__P2K__', '-DP2K', '-DFTR_BIG_ENDIAN', '-DFTR_E1'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			so_lib=[],
		),
		addresses=Recipe.RecipeAddresses(
			register=0x00000000, inject=0x00000000,
		),
	),

	'V3i_R4441D_G_08.01.03R' : Recipe(
		toolchain=P2K_SDK_GCC_ARM,
		flags=Recipe.RecipeFlags(
			cpu=['-mbig-endian', '-mthumb', '-mthumb-interwork', '-march=armv4t', '-mtune=arm7tdmi-s'],
			cpu_flavor=['-DFTR_ARM', '-DFTR_ARM7', '-DFTR_NEPTUNE', '-DFTR_NEPTUNE_LTE2'],
			build=['-D__P2K__', '-DP2K', '-DFTR_BIG_ENDIAN', '-DFTR_V3I'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			so_lib=[],
		),
		addresses=Recipe.RecipeAddresses(
			register=0x00000000, inject=0x00000000,
		),
	),

	'V3x_R252211LD_U_85.9B.E6P' : Recipe(
		toolchain=P2K_SDK_GCC_MCORE,
		flags=Recipe.RecipeFlags(
			cpu=['-mbig-endian', '-m4align', '-m340'],
			cpu_flavor=['-DFTR_MCORE', '-DFTR_MCORE340', '-DFTR_RAINBOW', '-DFTR_RAINBOW_POG'],
			build=['-D__P2K__', '-DP2K', '-DFTR_BIG_ENDIAN', '-DFTR_V3X'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			so_lib=[],
		),
		addresses=Recipe.RecipeAddresses(
			register=0x00000000, inject=0x00000000,
		),
	),

	'V3xx_R26111LD_U_96.A0.0ER' : Recipe(
		toolchain=P2K_SDK_GCC_ARM,
		flags=Recipe.RecipeFlags(
			cpu=['-mbig-endian', '-mbe32', '-mthumb', '-mthumb-interwork', '-march=armv6j', '-mtune=arm1136jf-s'],
			cpu_flavor=['-DFTR_ARM', '-DFTR_ARM11', '-DFTR_ARGON', '-DFTR_ARGON_LV'],
			build=['-D__P2K__', '-DP2K', '-DFTR_BIG_ENDIAN', '-DFTR_V3XX'],
			bin_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			elf_ldr=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL', '-DFTR_ALLOC_SUAPI'],
			so_lib=[],
		),
		addresses=Recipe.RecipeAddresses(
			register=0x00000000, inject=0x00000000,
		),
	),
}
