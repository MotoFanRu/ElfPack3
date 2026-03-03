from aleph import Flags
from aleph import Recipe
from aleph import Addresses

from aleph import P2K_SDK_GCC_ARM
from aleph import P2K_SDK_GCC_MCORE

RECIPES: dict[str, Recipe] = {

	'E1_R373_G_0E.30.49R' : Recipe(
		toolchain=P2K_SDK_GCC_ARM,
		flags=Flags(
			cpu=['-mbig-endian', '-march=armv4t', '-mtune=arm7tdmi-s', '-mthumb', '-mthumb-interwork'],
			build=['-DFTR_E1', '-DFTR_NEPTUNE', '-DFTR_NEPTUNE_LTE', '-DFTR_ARM', '-DFTR_ARM7'],
			feature=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL'],
			general=['-O2', '-fshort-wchar'],
		),
		addresses=Addresses(
			register=0x00000000,
			inject=0x00000000,
		),
	),

	'V3i_R4441D_G_08.01.03R' : Recipe(
		toolchain=P2K_SDK_GCC_ARM,
		flags=Flags(
			cpu=['-mbig-endian', '-march=armv4t', '-mtune=arm7tdmi-s', '-mthumb', '-mthumb-interwork'],
			build=['-DFTR_V3I', '-DFTR_NEPTUNE', '-DFTR_NEPTUNE_LTE2', '-DFTR_ARM', '-DFTR_ARM7'],
			feature=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL'],
			general=['-O2', '-fshort-wchar'],
		),
		addresses=Addresses(
			register=0x00000000,
			inject=0x00000000,
		),
	),

	'V3x_R252211LD_U_85.9B.E6P' : Recipe(
		toolchain=P2K_SDK_GCC_MCORE,
		flags=Flags(
			cpu=['-mbig-endian', '-m340', '-m4align'],
			build=['-DFTR_V3X', '-DFTR_RAINBOW', '-DFTR_RAINBOW_POG', '-DFTR_MCORE', '-DFTR_MCORE340'],
			feature=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL'],
			general=['-O2', '-fshort-wchar'],
		),
		addresses=Addresses(
			register=0x00000000,
			inject=0x00000000,
		),
	),

	'V3xx_R26111LD_U_96.A0.0ER' : Recipe(
		toolchain=P2K_SDK_GCC_ARM,
		flags=Flags(
			cpu=['-mbig-endian', '-mbe32', '-march=armv6j', '-mtune=arm1136jf-s', '-mthumb', '-mthumb-interwork'],
			build=['-DFTR_V3XX', '-DFTR_ARGON', '-DFTR_ARGON_LV', '-DFTR_ARM', '-DFTR_ARM11'],
			feature=['-DFTR_LOGGER', '-DFTR_LOGGER_JAL'],
			general=['-O2', '-fshort-wchar'],
		),
		addresses=Addresses(
			register=0x00000000,
			inject=0x00000000,
		),
	),
}
