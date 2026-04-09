import tomllib
from dataclasses import dataclass
from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_consts import AlephConsts
from .aleph_socs import AlephSoCs, SoC
from .aleph_toolchain import AlephToolchain, Toolchain

@dataclass
class Flags:
	build: list[str]
	bin_ldr: list[str]
	elf_ldr: list[str]

@dataclass
class Addresses:
	inject: int
	loader: int
	register: int
	task_a: int
	task_b: int

@dataclass
class Recipe:
	soc: SoC
	toolchain: Toolchain
	flags: Flags
	addresses: Addresses

class AlephRecipe:
	log: AlephLogger
	chk: AlephChecker
	consts: AlephConsts
	soc: AlephSoCs
	toolchain: AlephToolchain

	recipes: dict[str, Recipe]

	def __init__(
		self,
		logger: AlephLogger,
		checker: AlephChecker,
		consts: AlephConsts,
		soc: AlephSoCs,
		toolchain: AlephToolchain
	) -> None:
		self.log = logger
		self.chk = checker
		self.consts = consts
		self.soc = soc
		self.toolchain = toolchain
		self.recipes = { }

		self.parse_build_config(self.consts.P2K_BUILD_CONFIG)

	def parse_build_config(self, p_i: Path) -> bool:
		if not self.chk.check_files([p_i], CheckerMode.WARN):
			return False

		try:
			with p_i.open('rb') as build_config:
				build_config_data = tomllib.load(build_config)
				self.chk.is_ok_dict(build_config_data, CheckerMode.EXCEPTION)
		except Exception as e:
			self.log.C(f'Cannot parse build config file: {p_i}, {e}')
			return False

		for key, val in build_config_data.items():
			if not self.chk.is_key_present(val['specs']['soc'], self.soc.socs):
				self.log.E(f'Unknown SoC: {val["specs"]["soc"]}')
				return False
			if not self.chk.is_key_present(val['specs']['toolchain'], self.toolchain.toolchains):
				self.log.E(f'Unknown Toolchain: {val["specs"]["toolchain"]}')
				return False

			self.recipes[key] = Recipe(
				soc=self.soc.socs[val['specs']['soc']],
				toolchain=self.toolchain.toolchains[val['specs']['toolchain']],
				flags=Flags(
					build=val['flags']['general'].split(),
					bin_ldr=val['flags']['bin_ldr'].split(),
					elf_ldr=val['flags']['elf_ldr'].split(),
				),
				addresses=Addresses(
					inject=int(val['addrs']['inject']),
					loader=int(val['addrs']['loader']),
					register=int(val['addrs']['register']),
					task_a=int(val['addrs']['task_a']),
					task_b=int(val['addrs']['task_b']),
				),
			)

		return True
