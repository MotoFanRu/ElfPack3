# TODO: Recheck this on Ubuntu 26.04 LTS.
from __future__ import annotations

import sys
from typing import Callable
from datetime import datetime
from argparse import Namespace, RawDescriptionHelpFormatter

from .aleph_logger import AlephLogger
from .aleph_args import AlephArgs
from .aleph_datetime import AlephDateTime
from .aleph_checker import AlephChecker
from .aleph_filesystem import AlephFileSystem
from .aleph_invoker import AlephInvoker
from .aleph_consts import AlephConsts
from .aleph_socs import AlephSoCs
from .aleph_toolchain import AlephToolchain
from .aleph_recipe import AlephRecipe
from .aleph_stringer import AlephStringer
from .aleph_patcher import AlephPatcher
from .aleph_hexer import AlephHexer
from .aleph_model_def import AlephModelDef
from .aleph_format_def import AlephFormatDef
from .aleph_format_sym import AlephFormatSym
from .aleph_format_api import AlephFormatApi
from .aleph_format_asm import AlephFormatAsm
from .aleph_exception import AlephException
from .aleph_format_map import AlephFormatMap

class Aleph:
	log: AlephLogger
	args_parser: AlephArgs
	args: Namespace
	dt: AlephDateTime
	chk: AlephChecker
	fs: AlephFileSystem
	invoker: AlephInvoker
	consts: AlephConsts
	soc: AlephSoCs
	toolchain: AlephToolchain
	recipe: AlephRecipe
	strs: AlephStringer
	patch: AlephPatcher
	hex: AlephHexer
	mdef: AlephModelDef
	fdef: AlephFormatDef
	fsym: AlephFormatSym
	fapi: AlephFormatApi
	fasm: AlephFormatAsm
	fmap: AlephFormatMap

	def __init__(self) -> None:
		self.log = AlephLogger()
		self.log.configure('-v' in sys.argv or '--verbose' in sys.argv)
		self.args_parser = AlephArgs(formatter_class=RawDescriptionHelpFormatter)
		self.dt = AlephDateTime()
		self.chk = AlephChecker(self.log)
		self.fs = AlephFileSystem(self.log, self.chk)
		self.invoker = AlephInvoker(self.log, self.chk, self.fs)
		self.consts = AlephConsts()
		self.soc = AlephSoCs(self.consts)
		self.toolchain = AlephToolchain(self.log, self.chk, self.consts, self.invoker, self.fs)
		self.recipe = AlephRecipe(self.log, self.chk, self.consts, self.soc, self.toolchain)
		self.strs = AlephStringer(self.log, self.chk)
		self.patch = AlephPatcher(self.log, self.chk, self.fs)
		self.hex = AlephHexer(self.log, self.chk)
		self.mdef = AlephModelDef(self.log, self.chk, self.hex, self.strs)
		self.fdef = AlephFormatDef(self.log, self.chk, self.mdef, self.fs, self.strs, self.hex, self.patch)
		self.fsym = AlephFormatSym(self.log, self.chk, self.mdef, self.fs, self.strs, self.hex, self.patch)
		self.fapi = AlephFormatApi(self.log, self.chk, self.mdef, self.fs)
		self.fasm = AlephFormatAsm(self.log, self.chk, self.hex, self.mdef, self.patch, self.fs)
		self.fmap = AlephFormatMap(self.log, self.chk, self.mdef, self.fs, self.strs, self.hex, self.patch)

	def set_args(self, fill_args_fn: Callable[[Aleph], None], check_args_fn: Callable[[Aleph], None] = None) -> None:
		fill_args_fn(self)
		self.args = self.args_parser.parse_args()

		check_args_fn(self)

	def main(self, do_main_work_fn: Callable[[Aleph], bool]) -> bool:
		result = False

		start_time = datetime.now()
		self.log.D(f'Start on: {start_time}')

		try:
			result = do_main_work_fn(self)
		except AlephException as e:
			self.log.C(e)

		self.log.D(f'End on: {datetime.now()}')

		elapsed = self.dt.elapsed_format(datetime.now() - start_time)
		self.log.I(f'Elapsed: {elapsed}')

		return result
