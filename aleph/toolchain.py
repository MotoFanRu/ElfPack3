import os
import shutil

from pathlib import Path

from .invoker import invoke_run_input
from .logger import D
from .logger import I
from .logger import W
from .skeleton import Recipe
from .skeleton import Toolchain
from .skeleton import P2K_SDK_GCC
from .skeleton import P2K_SDK_INC

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

P2K_SDK_GCC_ARM = Toolchain(
	gcc=resolve_tool(P2K_SDK_GCC_ARM_BIN, 'arm-none-eabi-gcc')
)

P2K_SDK_GCC_MCORE = Toolchain(
	gcc=resolve_tool(P2K_SDK_GCC_MCORE_BIN, 'mcore-elf-gcc')
)

def compile_c(recipe: Recipe, p_in: Path, p_out: Path, custom_flags: list[str] | None = None) -> bool:
	I(f'Compiling "{p_in.name}" to "{p_out.name}"')
	D(f'"{p_in}" => "{p_out}"')

	command = [
		recipe.toolchain.gcc,
		'-I', str(P2K_SDK_INC),
		*recipe.flags.cpu,
		*recipe.flags.build,
		*recipe.flags.feature,
		*recipe.flags.general,
		*(custom_flags or []),
		'-c', p_in, '-o', p_out,
	]

	return invoke_run_input([p_in], command)
