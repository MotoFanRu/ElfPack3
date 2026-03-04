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

	W(f'Tool "{bundled_tool}" not found (bundled or PATH). Something might go wrong!')
	return Path(base_name)

P2K_SDK_COMPILER_FLAGS = Toolchain.CompilerFlags(
	general=['-std=c99', '-nostdinc'],
	warnings=['-Wall', '-Wextra', '-pedantic'],
	features=['-ffreestanding', '-fshort-wchar', '-fshort-enums', '-funsigned-char', '-fpack-struct=4'],
	optimization=['-O2'],
)

P2K_SDK_LINKER_FLAGS = Toolchain.LinkerFlags(
	general=['-s', '-nostdlib', '-Wl,--gc-sections'],
	bin_ldr=['-Wl,-eEP3_BIN_Loader_MainRegister'],
	elf_ldr=['-Z'],
	so_lib=['-Z'],
)

P2K_SDK_GCC_ARM = Toolchain(
	gcc=resolve_tool(P2K_SDK_GCC_ARM_BIN, 'arm-none-eabi-gcc'),
	cflags=P2K_SDK_COMPILER_FLAGS,
	lflags=P2K_SDK_LINKER_FLAGS
)

P2K_SDK_GCC_MCORE = Toolchain(
	gcc=resolve_tool(P2K_SDK_GCC_MCORE_BIN, 'mcore-elf-gcc'),
	cflags=P2K_SDK_COMPILER_FLAGS,
	lflags=P2K_SDK_LINKER_FLAGS
)

def gcc_compile(recipe: Recipe, p_in: Path, p_out: Path, custom_flags: list[str] | None = None) -> bool:
	I(f'Compiling "{p_in.name}" to "{p_out.name}"')
	D(f'"{p_in}" => "{p_out}"')

	command = [
		recipe.toolchain.gcc,
		'-I', str(P2K_SDK_INC),
		*recipe.toolchain.cflags.general,
		*recipe.toolchain.cflags.warnings,
		*recipe.toolchain.cflags.features,
		*recipe.toolchain.cflags.optimization,
		*recipe.flags.cpu,
		*recipe.flags.cpu_flavor,
		*recipe.flags.build,
		*(custom_flags or []),
		'-c', p_in, '-o', p_out,
	]

	return invoke_run_input([p_in], command)

def gcc_link(recipe: Recipe, p_in: list[Path], p_out: Path, custom_link_flags: list[str] | None = None) -> bool:
	I(f'Linking "{p_out.name}" from objects.')

	command = [
		recipe.toolchain.gcc,
		'-I', str(P2K_SDK_INC),
		*recipe.toolchain.cflags.general,
		*recipe.toolchain.cflags.warnings,
		*recipe.toolchain.cflags.features,
		*recipe.toolchain.cflags.optimization,
		*recipe.flags.cpu,
		*recipe.flags.cpu_flavor,
		*recipe.flags.build,
		*p_in,
		*recipe.toolchain.lflags.general,
		*(custom_link_flags or []),
		'-o', p_out
	]

	return invoke_run_input(p_in, command)
