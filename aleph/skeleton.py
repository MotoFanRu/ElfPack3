from enum import auto
from enum import StrEnum
from pathlib import Path
from dataclasses import dataclass

P2K_SDK_ROOT: Path = Path(__file__).resolve().parent.parent

P2K_SDK_ASM = P2K_SDK_ROOT / 'asm'
P2K_SDK_RES = P2K_SDK_ROOT / 'res'
P2K_SDK_INC = P2K_SDK_ROOT / 'inc'
P2K_SDK_LDS = P2K_SDK_ROOT / 'lds'
P2K_SDK_SRC = P2K_SDK_ROOT / 'src'
P2K_SDK_GCC = P2K_SDK_ROOT / 'gcc'
P2K_SDK_BUILD = P2K_SDK_ROOT / 'build'
P2K_SDK_RELEASE = P2K_SDK_ROOT / 'release'

# TODO: See P2kFeatures bla-bla-bla
EP3_BIN_LDR = 'ep3.bin'
EP3_BIN_FPA = 'ep3.fpa'
EP3_ELF_LDR = 'ep3.ldr'
EP3_ELF_LDR_LIB = 'ep3.so'
EP3_ELF_LDR_CFG = 'ep3.ini'
EP3_ELF_LDR_LOG = 'ep3.log'

@dataclass
class Toolchain:
	gcc: Path
	objcopy: Path
	nm: Path
	cflags: list[str]
	cflags_bin_ldr: list[str]
	cflags_elf_ldr: list[str]
	cflags_lib_so: list[str]
	lflags: list[str]
	lflags_bin_ldr: list[str]
	lflags_elf_ldr: list[str]
	lflags_so_lib: list[str]

class Endian(StrEnum):
	BIG = auto()
	LITTLE = auto()

@dataclass
class SoC:
	cpu: str
	end: Endian
	opt: list[str]
	asm: Path
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

@dataclass
class Definitions:
	@dataclass
	class Header:
		pfw: str
		ver: str
		cpu: str
	@dataclass
	class Definition:
		addr: int
		type: str
		name: str
	head: Header
	defs: list[Definition]
