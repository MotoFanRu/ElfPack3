import os
import shutil

from pathlib import Path

from .logger import D
from .logger import I
from .logger import W
from .skeleton import Recipe
from .skeleton import Toolchain
from .skeleton import P2K_SDK_GCC
from .skeleton import P2K_SDK_INC
from .invoker import invoke_run_input

IS_WINDOWS = os.name == 'nt'

P2K_SDK_GCC_ARM = P2K_SDK_GCC / ('arm-gcc-win' if IS_WINDOWS else 'arm-gcc-lin')
P2K_SDK_GCC_MCORE = P2K_SDK_GCC / ('mcore-gcc-win' if IS_WINDOWS else 'mcore-gcc-lin')
P2K_SDK_GCC_ARM_BIN = P2K_SDK_GCC_ARM / 'bin'
P2K_SDK_GCC_MCORE_BIN = P2K_SDK_GCC_MCORE / 'bin'

def resolve_tool(bin_dir: Path, base_name: str) -> Path:
	bundled_tool = bin_dir / (base_name + ('.exe' if IS_WINDOWS else ''))

	if bundled_tool.is_file():
		return bundled_tool

	if bundle_path := shutil.which(base_name):
		return Path(bundle_path)

	W(f'WARN Tool not found (bundled or PATH): "{bundled_tool}"')
	W('WARN Something might go wrong!')
	return Path(base_name)

GCC_CFLAGS_GENERAL = ['-std=c99', '-nostdinc']
GCC_CFLAGS_WARNINGS = ['-Wall', '-Wextra', '-pedantic']
GCC_CFLAGS_FEATURES = ['-ffreestanding', '-fshort-wchar', '-fshort-enums', '-funsigned-char', '-fpack-struct=4']
GCC_CFLAGS_OPTIMIZATIONS = ['-O2']
P2K_CFLAGS = ['-D__P2K__', '-DP2K']

GCC_CFLAGS = GCC_CFLAGS_GENERAL + GCC_CFLAGS_WARNINGS + GCC_CFLAGS_FEATURES + GCC_CFLAGS_OPTIMIZATIONS + P2K_CFLAGS

GCC_LFLAGS_GENERAL = ['-s', '-nostdlib', '-Wl,--gc-sections']

GCC_LFLAGS = GCC_LFLAGS_GENERAL

P2K_SDK_GCC_ARM = Toolchain(
	gcc=resolve_tool(P2K_SDK_GCC_ARM_BIN, 'arm-none-eabi-gcc'),
	objcopy=resolve_tool(P2K_SDK_GCC_ARM_BIN, 'arm-none-eabi-objcopy'),
	cflags=GCC_CFLAGS,
	lflags=GCC_LFLAGS,
)

P2K_SDK_GCC_MCORE = Toolchain(
	gcc=resolve_tool(P2K_SDK_GCC_MCORE_BIN, 'mcore-elf-gcc'),
	objcopy=resolve_tool(P2K_SDK_GCC_MCORE_BIN, 'mcore-elf-objcopy'),
	cflags=GCC_CFLAGS,
	lflags=GCC_LFLAGS
)

def gcc_compile(recipe: Recipe, p_in: Path, p_out: Path, custom_flags: list[str] | None = None) -> bool:
	I(f'Compiling "{p_in.name}" source.')
	D(f'"{p_in}" => "{p_out}"')

	command = [
		recipe.toolchain.gcc,
		'-I', str(P2K_SDK_INC),
		*recipe.toolchain.cflags,
		*recipe.soc.cflags,
		*recipe.flags.build,
		*(custom_flags or []),
		'-c', p_in, '-o', p_out,
	]

	return invoke_run_input([p_in], command)

def gcc_link(recipe: Recipe, p_in: list[Path], p_out: Path, custom_flags: list[str] | None = None) -> bool:
	I(f'Linking "{p_out.name}" from objects.')
	D(f'"{", ".join(p.name for p in p_in)}" => "{p_out.name}"')

	command = [
		recipe.toolchain.gcc,
		'-I', str(P2K_SDK_INC),
		*recipe.toolchain.cflags,
		*recipe.soc.cflags,
		*recipe.flags.build,
		*p_in,
		*recipe.toolchain.lflags,
		*(custom_flags or []),
		'-o', p_out
	]

	return invoke_run_input(p_in, command)

def gcc_bin(recipe: Recipe, p_in: Path, p_out: Path, custom_flags: list[str] | None = None) -> bool:
	I(f'Baking "{p_out.name}" binary.')
	D(f'"{p_in}" => "{p_out}"')

	command = [
		recipe.toolchain.objcopy,
		'-O', 'binary',
		'-j', '.text*',
		*(custom_flags or []),
		p_in, p_out
	]

	return invoke_run_input([p_in], command)
