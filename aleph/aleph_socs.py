from enum import Enum
from dataclasses import dataclass
from pathlib import Path

from .aleph_consts import AlephConsts

class Endian(Enum):
	LITTLE = 0
	BIG    = 1

@dataclass
class SoC:
	cpu: str
	end: Endian
	opt: list[str]
	asm: Path
	lds: Path

class AlephSoCs:
	CPU_ARCH = ['ARM7', 'ARM9', 'ARM11', 'MCORE210', 'MCORE340', 'NONE']
	CPU_ARM = ['-DFTR_ARM']
	CPU_ARM_MODE_ARM = ['-marm', '-mthumb-interwork', '-DFTR_ARM_MODE']
	CPU_ARM_MODE_THUMB = ['-mthumb', '-mthumb-interwork', '-DFTR_THUMB_MODE']
	CPU_ARM_LITTLE_ENDIAN = ['-mlittle-endian', '-DFTR_LITTLE_ENDIAN']
	CPU_ARM7_BIG_ENDIAN = ['-mbig-endian', '-DFTR_BIG_ENDIAN']
	CPU_ARM11_BIG_ENDIAN = ['-mbig-endian', '-mbe32', '-DFTR_BIG_ENDIAN']
	CPU_ARM7_FLAVOR = ['-march=armv4t', '-mtune=arm7tdmi-s', '-DFTR_ARM7']
	CPU_ARM9_FLAVOR = ['-march=armv5te', '-mtune=arm926ej-s', '-DFTR_ARM9']
	CPU_ARM11_FLAVOR = ['-march=armv6j', '-mtune=arm1136jf-s', '-DFTR_ARM11']

	CPU_MCORE = ['-DFTR_MCORE']
	CPU_MCORE_BIG_ENDIAN = ['-mbig-endian']
	CPU_MCORE210_FLAVOR = ['-m210', '-DFTR_MCORE210']  # GCC M-CORE Flags '-m4align', '-m8align' are deprecated.
	CPU_MCORE340_FLAVOR = ['-m340', '-DFTR_MCORE340']  # GCC M-CORE Flags '-m4align', '-m8align' are deprecated.

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

	consts: AlephConsts

	socs: dict[str, SoC]

	def __init__(self, consts: AlephConsts) -> None:
		self.consts = consts

		self.socs = {
			'WALLY': SoC(
				cpu='MCORE210',
				end=Endian.BIG,
				opt=self.WALLY_FLAGS,
				asm=self.consts.P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Wally.lds',
			),
			'PATRIOT': SoC(
				cpu='MCORE210',
				end=Endian.BIG,
				opt=self.PATRIOT_FLAGS,
				asm=self.consts.P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Patriot.lds',
			),
			'RAINBOW': SoC(
				cpu='MCORE340',
				end=Endian.BIG,
				opt=self.RAINBOW_FLAGS,
				asm=self.consts.P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Rainbow.lds',
			),
			'RAINBOW_POG': SoC(
				cpu='MCORE340',
				end=Endian.BIG,
				opt=self.RAINBOW_FLAGS + ['-DFTR_RAINBOW_POG'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_MCORE.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Rainbow_PoG.lds',
			),
			'NEPTUNE_LT_LCA': SoC(
				cpu='ARM7',
				end=Endian.BIG,
				opt=self.NEPTUNE_FLAGS + ['-DFTR_NEPTUNE_LT_LCA'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Neptune_LT_LCA.lds',
			),
			'NEPTUNE_LTS': SoC(
				cpu='ARM7',
				end=Endian.BIG,
				opt=self.NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTS'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Neptune_LTS.lds',
			),
			'NEPTUNE_ULS': SoC(
				cpu='ARM7',
				end=Endian.BIG,
				opt=self.NEPTUNE_FLAGS + ['-FTR_NEPTUNE_ULS'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Neptune_ULS.lds',
			),
			'NEPTUNE_LTE': SoC(
				cpu='ARM7',
				end=Endian.BIG,
				opt=self.NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTE'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Neptune_LTE.lds',
			),
			'NEPTUNE_LTE2': SoC(
				cpu='ARM7',
				end=Endian.BIG,
				opt=self.NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTE2'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Neptune_LTE2.lds',
			),
			'NEPTUNE_LTE2_ROM4': SoC(
				cpu='ARM7',
				end=Endian.BIG,
				opt=self.NEPTUNE_FLAGS + ['-FTR_NEPTUNE_LTE2_ROM4'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Neptune_LTE2_ROM4.lds',
			),
			'ARGON_PLUS': SoC(
				cpu='ARM11',
				end=Endian.BIG,
				opt=self.ARGON_FLAGS + ['-DFTR_ARGON_PLUS'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Argon_Plus.lds',
			),
			'ARGON_LV': SoC(
				cpu='ARM11',
				end=Endian.BIG,
				opt=self.ARGON_FLAGS + ['-DFTR_ARGON_LV'],
				asm=self.consts.P2K_SDK_ASM / 'Defs_THUMB_ARM.tpl.S',
				lds=self.consts.P2K_SDK_LDS / 'SoC_Argon_LV.lds',
			)
		}
