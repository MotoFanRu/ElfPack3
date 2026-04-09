from enum import auto, StrEnum
from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_hexer import AlephHexer
from .aleph_model_def import AlephModelDef, ModelDef, SymbolDef
from .aleph_patcher import AlephPatcher
from .aleph_filesystem import AlephFileSystem
from .aleph_exception import AlephParserException

class AsmMode(StrEnum):
	ARM_ASM = auto()
	MCORE_ASM = auto()

class AlephFormatAsm:
	ASM_EXT = ['.asm', '.s', '.S']

	log: AlephLogger
	chk: AlephChecker
	hex: AlephHexer
	mdef: AlephModelDef
	patch: AlephPatcher
	fs: AlephFileSystem

	def __init__(
		self,
		logger: AlephLogger,
		checker: AlephChecker,
		hexer: AlephHexer,
		mdef: AlephModelDef,
		patcher: AlephPatcher,
		filesystem: AlephFileSystem
	) -> None:
		self.log = logger
		self.chk = checker
		self.hex = hexer
		self.mdef = mdef
		self.patch = patcher
		self.fs = filesystem

	def parse_equ_line(self, line: str, mode: AsmMode) -> SymbolDef | None:
		line = line.strip()
		if line.startswith('.equ ') or line.startswith('.set '):
			rest = line[5:].split(';', 1)[0].strip()
			if ',' in rest:
				asm_flavor = 'M' if mode == AsmMode.MCORE_ASM else 'A'
				func_name = rest.split(',', 1)[0].strip()
				func_addr = rest.split(',', 1)[1].strip().split()[0]
				func_type = 'T' if rest.split(',', 1)[1].strip().endswith('| 1') else asm_flavor

				if not self.hex.is_align(func_addr, 2):
					self.log.W(f'Not aligned by 2, will be treated as D (data address): {line}')
					func_type = 'D'

				if func_name:
					return SymbolDef(
						name=func_name,
						type=func_type,
						addr=self.hex.u32i(func_addr),
					)
		return None

	def generate_asm_line(self, max_len: int, def_type: str, def_name: str, def_val: str) -> str:
		self.chk.is_ok_str(def_type, CheckerMode.EXCEPTION)
		self.chk.is_ok_str(def_name, CheckerMode.EXCEPTION)
		self.chk.is_ok_str(def_val, CheckerMode.EXCEPTION)

		if def_type == 'C':
			def_type = 'D'

		return f'def_{def_type} {def_name + ",":<{max_len}} {def_val}'

	def generate_asm(self, p_asm_template: Path, model: ModelDef) -> str:
		template = self.fs.read_text_file(p_asm_template)
		self.chk.is_ok_str(template, CheckerMode.EXCEPTION)

		phone, firmware = model.head.pfw.split('_', 1)
		version = model.head.ver

		max_name_len = max((len(d.name) for d in model.syms), default=0) + len('EP3_firmware') + 4

		listing = [
			self.generate_asm_line(max_name_len, 'S', 'EP3_phone', f'"{phone}"'),
			self.generate_asm_line(max_name_len, 'S', 'EP3_firmware', f'"{firmware}"'),
			self.generate_asm_line(max_name_len, 'S', 'EP3_version', f'"{version}"')
		]

		for s in model.syms:
			listing.append(self.generate_asm_line(max_name_len, s.type, s.name, self.hex.u32s(s.addr)))

		markers = ['%ENTITY_LISTING%']
		patches = [''.join(map(lambda p: f'\t{p}\n', listing))]
		patch = self.patch.patch_text(markers, patches, template)

		self.chk.is_ok_str(patch, CheckerMode.EXCEPTION)

		return patch + '\n'

	def write_asm(self, p_asm_template: Path, p_o: Path, model: ModelDef) -> bool:
		self.chk.check_ext(p_o, self.ASM_EXT, CheckerMode.EXCEPTION)

		if not self.mdef.is_def_model(model):
			self.log.E(f'Model is wrong!')
			return False

		if not self.mdef.check_addrs_names(model.syms, True):
			return False

		return self.fs.write_text_file(p_o, self.generate_asm(p_asm_template, model))

	def read_asm(self, p_i: Path, mode: AsmMode) -> list[SymbolDef]:
		self.chk.check_ext(p_i, self.ASM_EXT, CheckerMode.EXCEPTION)
		lines = self.fs.read_text_lines(p_i)
		self.chk.is_ok_list(lines, CheckerMode.EXCEPTION)

		symbols = []
		for line in lines:
			symbol = self.parse_equ_line(line, mode)
			if symbol:
				if not self.mdef.is_symbol(symbol, line):
					raise AlephParserException(f'Parse error on line: {line}')
				symbols.append(symbol)

		if not self.chk.is_ok_list(symbols):
			raise AlephParserException('Symbols list is not valid or empty!')

		symbols = self.mdef.sort_symbols(symbols)

		return symbols
