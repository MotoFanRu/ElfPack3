from pathlib import Path
from dataclasses import dataclass

P2K_SDK_ROOT: Path = Path(__file__).resolve().parent.parent

P2K_SDK_BUILD = P2K_SDK_ROOT / 'build'
P2K_SDK_RES = P2K_SDK_ROOT / 'res'
P2K_SDK_INC = P2K_SDK_ROOT / 'inc'
P2K_SDK_SRC = P2K_SDK_ROOT / 'src'
P2K_SDK_TOOL = P2K_SDK_ROOT / 'tool'
P2K_SDK_GCC = P2K_SDK_TOOL / 'gcc'

@dataclass
class Toolchain:
	@dataclass
	class CompilerFlags:
		general: list[str]
		warnings: list[str]
		features: list[str]
		optimization: list[str]
	@dataclass
	class LinkerFlags:
		general: list[str]
		bin_ldr: list[str]
		elf_ldr: list[str]
		so_lib: list[str]
	gcc: Path
	cflags: CompilerFlags
	lflags: LinkerFlags

@dataclass
class Recipe:
	@dataclass
	class RecipeFlags:
		cpu: list[str]
		cpu_flavor: list[str]
		build: list[str]
		bin_ldr: list[str]
		elf_ldr: list[str]
		so_lib: list[str]
	@dataclass
	class RecipeAddresses:
		register: int
		inject: int
	toolchain: Toolchain
	flags: RecipeFlags
	addresses: RecipeAddresses
