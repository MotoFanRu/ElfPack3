from pathlib import Path

from .skeleton import SoC
from .skeleton import Endian
from .skeleton import P2K_SDK_LDS
from .skeleton import P2K_SDK_ASM

CPU_ARCH = ['ARM7', 'ARM9', 'ARM11', 'MCORE210', 'MCORE340', 'NONE']

CPU_ARM = ['-DFTR_ARM']
CPU_ARM_MODE_ARM = ['-marm', '-mthumb-interwork']
CPU_ARM_MODE_THUMB = ['-mthumb', '-mthumb-interwork']
CPU_ARM_LITTLE_ENDIAN = ['-mlittle-endian', '-DFTR_LITTLE_ENDIAN']
CPU_ARM7_BIG_ENDIAN = ['-mbig-endian', '-DFTR_BIG_ENDIAN']
CPU_ARM11_BIG_ENDIAN = ['-mbig-endian', '-mbe32', '-DFTR_BIG_ENDIAN']
CPU_ARM7_FLAVOR = ['-march=armv4t', '-mtune=arm7tdmi-s', '-DFTR_ARM7']
CPU_ARM9_FLAVOR = ['-march=armv5te', '-mtune=arm926ej-s', '-DFTR_ARM9']
CPU_ARM11_FLAVOR = ['-march=armv6j', '-mtune=arm1136jf-s', '-DFTR_ARM11']

CPU_MCORE = ['-DFTR_MCORE']
CPU_MCORE_BIG_ENDIAN = ['-mbig-endian']
CPU_MCORE210_FLAVOR = ['-m210', '-m4align', '-DFTR_MCORE210']
CPU_MCORE340_FLAVOR = ['-m340', '-m4align', '-DFTR_MCORE340']

FTR_WALLY = ['-DFTR_WALLY']
FTR_PATRIOT = ['-DFTR_PATRIOT']
FTR_RAINBOW = ['-DFTR_RAINBOW']
FTR_ARGON = ['-DFTR_ARGON']
FTR_NEPTUNE = ['-DFTR_NEPTUNE']

WALLY_FLAGS = CPU_MCORE + CPU_MCORE210_FLAVOR + CPU_MCORE_BIG_ENDIAN + FTR_WALLY
PATRIOT_FLAGS = CPU_MCORE + CPU_MCORE210_FLAVOR + CPU_MCORE_BIG_ENDIAN + FTR_PATRIOT
RAINBOW_FLAGS = CPU_MCORE + CPU_MCORE340_FLAVOR + CPU_MCORE_BIG_ENDIAN + FTR_RAINBOW
NEPTUNE_FLAGS = CPU_ARM + CPU_ARM_MODE_THUMB + CPU_ARM7_BIG_ENDIAN + CPU_ARM7_FLAVOR + FTR_NEPTUNE
ARGON_FLAGS = CPU_ARM + CPU_ARM_MODE_THUMB + CPU_ARM11_BIG_ENDIAN + CPU_ARM11_FLAVOR + FTR_ARGON

SOC_WALLY = SoC(
	cpu='MCORE210',
	end=Endian.BIG,
	opt=WALLY_FLAGS,
	asm=P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Wally.lds',
)

SOC_PATRIOT = SoC(
	cpu='MCORE210',
	end=Endian.BIG,
	opt=PATRIOT_FLAGS,
	asm=P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Patriot.lds',
)

SOC_RAINBOW = SoC(
	cpu='MCORE340',
	end=Endian.BIG,
	opt=RAINBOW_FLAGS,
	asm=P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Rainbow.lds',
)

SOC_RAINBOW_POG = SoC(
	cpu='MCORE340',
	end=Endian.BIG,
	opt=RAINBOW_FLAGS + ['-DFTR_RAINBOW_POG'],
	asm=P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Rainbow_PoG.lds',
)

SOC_NEPTUNE_LT_LCA = SoC(
	cpu='ARM7',
	end=Endian.BIG,
	opt=NEPTUNE_FLAGS + ['-DFTR_NEPTUNE_LT_LCA'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Neptune_LT_LCA.lds',
)

SOC_NEPTUNE_LTS = SoC(
	cpu='ARM7',
	end=Endian.BIG,
	opt=NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTS'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Neptune_LTS.lds',
)

SOC_NEPTUNE_ULS = SoC(
	cpu='ARM7',
	end=Endian.BIG,
	opt=NEPTUNE_FLAGS + ['-FTR_NEPTUNE_ULS'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Neptune_ULS.lds',
)

SOC_NEPTUNE_LTE = SoC(
	cpu='ARM7',
	end=Endian.BIG,
	opt=NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTE'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Neptune_LTE.lds',
)

SOC_NEPTUNE_LTE2 = SoC(
	cpu='ARM7',
	end=Endian.BIG,
	opt=NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTE2'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Neptune_LTE2.lds',
)

SOC_NEPTUNE_LTE2_ROM4 = SoC(
	cpu='ARM7',
	end=Endian.BIG,
	opt=NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTE2_ROM4'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Neptune_LTE2_ROM4.lds',
)

SOC_ARGON_PLUS = SoC(
	cpu='ARM11',
	end=Endian.BIG,
	opt=ARGON_FLAGS + ['-DFTR_ARGON_PLUS'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Argon_Plus.lds',
)

SOC_ARGON_LV = SoC(
	cpu='ARM11',
	end=Endian.BIG,
	opt=ARGON_FLAGS + ['-DFTR_ARGON_LV'],
	asm=P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
	lds=P2K_SDK_LDS / 'SoC_Argon_LV.lds',
)
