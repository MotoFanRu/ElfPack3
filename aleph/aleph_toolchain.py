import os
import shutil
from dataclasses import dataclass
from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_consts import AlephConsts
from .aleph_invoker import AlephInvoker
from .aleph_filesystem import AlephFileSystem

@dataclass
class Toolchain:
	bin_dir: Path
	gcc: str
	objcopy: str
	nm: str
	ar: str
	strip: str
	cflags: list[str]
	cflags_bin_ldr: list[str]
	cflags_elf_ldr: list[str]
	cflags_lib_so: list[str]
	lflags: list[str]
	lflags_bin_ldr: list[str]
	lflags_elf_ldr: list[str]
	lflags_so_lib: list[str]

class AlephToolchain:
	IS_WINDOWS: bool = os.name == 'nt'

	P2K_SDK_GCC_ARM: Path = AlephConsts.P2K_SDK_GCC / ('arm-gcc-win' if IS_WINDOWS else 'arm-gcc-lin')
	P2K_SDK_GCC_MCORE: Path = AlephConsts.P2K_SDK_GCC / ('mcore-gcc-win' if IS_WINDOWS else 'mcore-gcc-lin')
	P2K_SDK_GCC_ARM_BIN: Path = P2K_SDK_GCC_ARM / 'bin'
	P2K_SDK_GCC_MCORE_BIN: Path = P2K_SDK_GCC_MCORE / 'bin'

	GCC_CFLAGS_GENERAL = ['-std=c99', '-nostdinc']
	GCC_CFLAGS_WARNINGS = ['-Wall', '-Wextra', '-pedantic']
	GCC_CFLAGS_FEATURES = ['-ffreestanding', '-funsigned-char', '-fshort-enums', '-fshort-wchar', '-fpack-struct=4']
	GCC_CFLAGS_SECTIONS = ['-ffunction-sections', '-fdata-sections']  # It's better do not use it with M-CORE.
	GCC_CFLAGS_OPTIMIZATIONS = ['-O2']
	P2K_CFLAGS = ['-D__P2K__', '-DP2K']

	GCC_CFLAGS = GCC_CFLAGS_GENERAL + GCC_CFLAGS_WARNINGS + GCC_CFLAGS_FEATURES + GCC_CFLAGS_OPTIMIZATIONS + P2K_CFLAGS
	GCC_CFLAGS_BIN_LDR = ['-DFTR_BIN_LDR']
	GCC_CFLAGS_ELF_LDR = ['-DFTR_ELF_LDR']
	GCC_CFLAGS_SO_LIB  = ['-DFTR_SO_LIB', '-fPIC']

	GCC_LFLAGS_GENERAL = ['-nostdlib', '-Wl,--gc-sections']
	# Stripping is not good for generating map listings by `nm` calls. No symbols with it.
	GCC_LFLAGS_STRIP = ['-s']
	GCC_LFLAGS = GCC_LFLAGS_GENERAL
	GCC_LFLAGS_BIN_LDR = []
	GCC_LFLAGS_ELF_LDR = []
	GCC_LFLAGS_SO_LIB  = ['-shared']

	log: AlephLogger
	chk: AlephChecker
	consts: AlephConsts
	invoker: AlephInvoker
	fs: AlephFileSystem

	toolchains: dict[str, Toolchain]

	def __init__(
		self,
		logger: AlephLogger,
		checker: AlephChecker,
		consts: AlephConsts,
		invoker: AlephInvoker,
		filesystem: AlephFileSystem
	) -> None:
		self.log = logger
		self.chk = checker
		self.consts = consts
		self.invoker = invoker
		self.fs = filesystem

		self.toolchains = {
			'GCC_ARM': Toolchain(
				bin_dir=self.P2K_SDK_GCC_ARM_BIN,
				gcc='arm-none-eabi-gcc',
				objcopy='arm-none-eabi-objcopy',
				nm='arm-none-eabi-nm',
				ar='arm-none-eabi-ar',
				strip='arm-none-eabi-strip',
				cflags=self.GCC_CFLAGS,
				cflags_bin_ldr=self.GCC_CFLAGS_BIN_LDR,
				cflags_elf_ldr=self.GCC_CFLAGS_ELF_LDR,
				cflags_lib_so=self.GCC_CFLAGS_SO_LIB,
				lflags=self.GCC_LFLAGS,
				lflags_bin_ldr=self.GCC_LFLAGS_BIN_LDR,
				lflags_elf_ldr=self.GCC_LFLAGS_ELF_LDR,
				lflags_so_lib=self.GCC_LFLAGS_SO_LIB,
			),
			'GCC_MCORE': Toolchain(
				bin_dir=self.P2K_SDK_GCC_MCORE_BIN,
				gcc='mcore-elf-gcc',
				objcopy='mcore-elf-objcopy',
				nm='mcore-elf-nm',
				ar='mcore-elf-ar',
				strip='mcore-elf-strip',
				cflags=self.GCC_CFLAGS,
				cflags_bin_ldr=self.GCC_CFLAGS_BIN_LDR,
				cflags_elf_ldr=self.GCC_CFLAGS_ELF_LDR,
				cflags_lib_so=self.GCC_CFLAGS_SO_LIB,
				lflags=self.GCC_LFLAGS,
				lflags_bin_ldr=self.GCC_LFLAGS_BIN_LDR,
				lflags_elf_ldr=self.GCC_LFLAGS_ELF_LDR,
				lflags_so_lib=self.GCC_LFLAGS_SO_LIB,
			),
		}

	def resolve_tool(self, p_d: Path, base_name: str) -> Path:
		bundled_tool = p_d / (base_name + ('.exe' if self.IS_WINDOWS else ''))

		if bundled_tool.is_file():
			return bundled_tool

		if bundle_path := shutil.which(base_name):
			return Path(bundle_path)

		self.log.W(f'Tool not found (bundled or PATH): {bundled_tool}')
		return Path(base_name)

	def gcc_cc(self, toolchain: Toolchain, p_i: Path, p_o: Path, custom_cflags: list[str] | None = None) -> bool:
		self.log.I(f'  Compile: {p_i.name}')
		self.log.D(f'{p_i} => {p_o}')

		gcc_cc_prog = self.resolve_tool(toolchain.bin_dir, toolchain.gcc)

		command = [
			gcc_cc_prog,
			'-I', str(self.consts.P2K_SDK_INC),
			*toolchain.cflags,
			*(custom_cflags or []),
			'-c', p_i, '-o', p_o,
		]

		return self.invoker.run_cmd_input([p_i], command)

	def gcc_ld(
			self,
			toolchain: Toolchain,
			p_is: list[Path],
			p_o: Path,
			custom_cflags: list[str] | None = None,
			custom_lflags: list[str] | None = None
	) -> bool:
		self.log.I(f'  Link: {p_o.name}')
		self.log.D(f'[{", ".join(p.name for p in p_is)}] => {p_o.name}')

		gcc_ld_prog = self.resolve_tool(toolchain.bin_dir, toolchain.gcc)

		command = [
			gcc_ld_prog,
			'-I', str(self.consts.P2K_SDK_INC),
			*toolchain.cflags,
			*(custom_cflags or []),
			*p_is,
			*toolchain.lflags,
			*(custom_lflags or []),
			'-o', p_o
		]

		return self.invoker.run_cmd_input(p_is, command)

	def gcc_bin(self, toolchain: Toolchain, p_i: Path, p_o: Path, custom_flags: list[str] | None = None) -> bool:
		self.log.I(f'  Binary: {p_o.name}')
		self.log.D(f'{p_i} => {p_o}')

		gcc_objcopy_prog = self.resolve_tool(toolchain.bin_dir, toolchain.objcopy)

		command = [
			gcc_objcopy_prog,
			'-O', 'binary',
			'-j', '.text',
			*(custom_flags or []),
			p_i, p_o
		]

		return self.invoker.run_cmd_input([p_i], command)

	def gcc_nm(self, toolchain: Toolchain, p_i: Path, p_o: Path, custom_flags: list[str] | None = None) -> bool:
		self.log.I(f'  Symbols: {p_i.name}')
		self.log.D(f'{p_i} => {p_o}')

		gcc_nm_prog = self.resolve_tool(toolchain.bin_dir, toolchain.nm)

		command = [
			gcc_nm_prog,
			*(custom_flags or []),
			p_i
		]

		status = self.invoker.run_cmd_input([p_i], command, p_o)
		content = self.fs.read_text_file(p_o)
		if not self.chk.is_ok_str(content, CheckerMode.WARN):
			return False

		self.log.D(f'{content}')

		return status

	def gcc_ar(self, toolchain: Toolchain, p_is: list[Path], p_o: Path, custom_flags: list[str] | None = None) -> bool:
		self.log.I(f'  Archive: {p_o.name}')
		self.log.D(f'[{", ".join(p.name for p in p_is)}] => {p_o.name}')

		gcc_ar_prog = self.resolve_tool(toolchain.bin_dir, toolchain.ar)

		command = [
			gcc_ar_prog,
			*(custom_flags or []),
			'rcs',
			p_o,
			*p_is
		]

		return self.invoker.run_cmd_input(p_is, command)

	def gcc_strip(self, toolchain: Toolchain, p_io: Path, custom_flags: list[str] | None = None) -> bool:
		self.log.I(f'  Strip: {p_io.name}')
		self.log.D(f'{p_io} => {p_io}')

		gcc_strip_prog = self.resolve_tool(toolchain.bin_dir, toolchain.strip)

		command = [
			gcc_strip_prog,
			'-s',
			*(custom_flags or []),
			p_io
		]

		return self.invoker.run_cmd_input([p_io], command)
