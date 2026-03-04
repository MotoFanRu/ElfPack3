from pathlib import Path
from dataclasses import dataclass

P2K_SDK_ROOT: Path = Path(__file__).resolve().parent.parent

P2K_SDK_BUILD = P2K_SDK_ROOT / 'build'
P2K_SDK_RES = P2K_SDK_ROOT / 'res'
P2K_SDK_INC = P2K_SDK_ROOT / 'inc'
P2K_SDK_LDS = P2K_SDK_ROOT / 'lds'
P2K_SDK_SRC = P2K_SDK_ROOT / 'src'
P2K_SDK_TOOL = P2K_SDK_ROOT / 'tool'
P2K_SDK_GCC = P2K_SDK_TOOL / 'gcc'

@dataclass
class Toolchain:
	gcc: Path
	objcopy: Path
	cflags: list[str]
	lflags: list[str]

@dataclass
class SoC:
	cflags: list[str]
	lds: Path

@dataclass
class Recipe:
	@dataclass
	class RecipeFlags:
		build: list[str]
		bin_ldr: list[str]
		elf_ldr: list[str]
	@dataclass
	class RecipeAddresses:
		register: int
		inject: int
	soc: SoC
	toolchain: Toolchain
	flags: RecipeFlags
	addresses: RecipeAddresses
