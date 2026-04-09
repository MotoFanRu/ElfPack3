from dataclasses import dataclass
from datetime import datetime

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_socs import AlephSoCs
from .aleph_hexer import AlephHexer
from .aleph_stringer import AlephStringer
from .aleph_datetime import AlephDateTime
from .aleph_exception import AlephParserException

@dataclass
class HeaderDef:
	pfw: str
	ver: str
	cpu: str

@dataclass
class SymbolDef:
	addr: int
	type: str
	name: str

@dataclass
class ModelDef:
	head: HeaderDef
	syms: list[SymbolDef]

class AlephModelDef:
	log: AlephLogger
	chk: AlephChecker
	hex: AlephHexer
	strs: AlephStringer

	DEF_TYPES = ['A', 'T', 'D', 'C', 'M']

	def __init__(self, logger: AlephLogger, checker: AlephChecker, hexer: AlephHexer, stringer: AlephStringer) -> None:
		self.log = logger
		self.chk = checker
		self.hex = hexer
		self.strs = stringer

	def is_header(self, head: HeaderDef | None) -> bool:
		if not head:
			self.log.E('Header is Null or Empty.')
			return False

		if not self.chk.is_ok_str(head.pfw):
			self.log.E('PFW (phone and firmware) header string is Null or Empty.')
			return False
		if not self.chk.is_ok_str(head.ver):
			self.log.E('VER (version) header string is Null or Empty.')
			return False
		if not self.chk.is_ok_str(head.cpu):
			self.log.E('CPU (architecture) header string is Null or Empty.')
			return False

		if len(head.pfw.split('_', 1)) != 2:
			self.log.E(f'PFW (phone and firmware) header string is wrong: {head.pfw}')
			self.log.E('Should be in "C650_R365_G_0B.D3.08R" format.')
			return False

		if (len(head.ver.split('_', 1)) != 2) and (not head.ver.startswith('EP3_')):
			self.log.E(f'VER (version) header string is wrong: {head.ver}')
			self.log.E('Should be in "EP3_01-Feb-2025" format.')
			return False

		if head.cpu not in AlephSoCs.CPU_ARCH:
			self.log.E(f'CPU (architecture) header string is wrong: {head.cpu}')
			self.log.E(f'Should be in [{", ".join(AlephSoCs.CPU_ARCH)}] scope.')
			return False

		return True

	@staticmethod
	def type_align(def_type: str) -> int:
		if def_type in ['A']:
			return 4
		elif def_type in ['T', 'M']:
			return 2
		else:
			return 0

	def is_symbol(self, symbol: SymbolDef | None, line: str | None = None) -> bool:
		if not symbol:
			self.log.E('Symbol is Null or Empty.')
			if self.chk.is_ok_str(line):
				self.log.E(f'Line: {line}')
			return False

		if not self.hex.is_u32(symbol.addr):
			if self.chk.is_ok_str(line):
				self.log.E(f'Line: {line}')
			return False

		align = self.type_align(symbol.type)
		if not self.hex.is_align(symbol.addr, align):
			if self.chk.is_ok_str(line):
				self.log.E(f'Line: {line}')
			return False

		if symbol.type not in self.DEF_TYPES:
			self.log.E(f'Unknown type: {symbol.type}, {symbol.name}')
			if self.chk.is_ok_str(line):
				self.log.E(f'Line: {line}')
			return False

		if len(symbol.name) > 80:
			self.log.E(f'Too long name, more than 80 characters: {symbol.name}')
			if self.chk.is_ok_str(line):
				self.log.E(f'Line: {line}')
		return True

	def is_def_model(self, model: ModelDef | None) -> bool:
		if not model:
			self.log.E('Model is Null or Empty.')
			return False

		if not self.is_header(model.head):
			self.log.E('Header is wrong.')
			return False

		for sym in model.syms:
			if not self.is_symbol(sym):
				self.log.E(f'Symbol is wrong: {sym}')
				return False

		return True

	@staticmethod
	def syms_by_addr(syms: list[SymbolDef], addr: int) -> list[SymbolDef]:
		return [s for s in syms if s.addr == addr]

	@staticmethod
	def syms_by_name(syms: list[SymbolDef], name: str) -> list[SymbolDef]:
		return [s for s in syms if s.name == name]

	@staticmethod
	def syms_by_types(syms: list[SymbolDef], types: list[str]) -> list[SymbolDef]:
		return [s for s in syms if s.type in types]

	def check_addrs_names(
			self,
			symbols: list[SymbolDef],
			warn_on_duplicates: bool = False,
			skip_const_data: bool = False,
	) -> bool:
		if not self.chk.is_ok_list(symbols, CheckerMode.WARN):
			return False

		addrs = []
		names = []
		for sym in symbols:
			if not skip_const_data or sym not in {'D', 'C'}:
				addrs.append(sym.addr)
				names.append(sym.name)

		log = self.log.W if warn_on_duplicates else self.log.D

		addrs_d = self.strs.find_duplicates(addrs)
		if addrs_d and self.chk.is_ok_list(addrs_d):
			log('Duplicate addresses found:')
			for addr in addrs_d:
				log('')
				for s in self.syms_by_addr(symbols, addr):
					log(f'{self.hex.u32s(s.addr)} {s.type} {s.name}')

		names_d = self.strs.find_duplicates(names)
		if names_d and len(names_d) > 0:
			self.log.E('Duplicate names found:')
			for name in names_d:
				self.log.E('')
				for s in self.syms_by_name(symbols, name):
					self.log.E(f'{self.hex.u32s(s.addr)} {s.type} {s.name}')
			return False

		return True

	def sort_symbols(self, symbols: list[SymbolDef], sort_by_addrs: bool = False) -> list[SymbolDef]:
		self.chk.is_ok_list(symbols, CheckerMode.EXCEPTION)

		consts = self.syms_by_types(symbols, ['C'])
		addrs = self.syms_by_types(symbols, ['D'])
		funcs = self.syms_by_types(symbols, ['A', 'T', 'M'])

		sort_key = (lambda e: e.addr) if sort_by_addrs else (lambda e: e.name.lower())
		for target_list in [consts, addrs, funcs]:
			target_list.sort(key=sort_key)

		sorted_symbols = consts + addrs + funcs
		self.chk.is_ok_list(sorted_symbols, CheckerMode.EXCEPTION)
		return sorted_symbols

	@staticmethod
	def gen_ver() -> str:
		return 'EP3_' + AlephDateTime.date_format(datetime.now())

	@staticmethod
	def gen_header() -> HeaderDef:
		return HeaderDef(
			pfw='PHONE_FIRMWARE',
			ver=AlephModelDef.gen_ver(),
			cpu='NONE'
		)

	@staticmethod
	def gen_model() -> ModelDef:
		return ModelDef(
			head=AlephModelDef.gen_header(),
			syms=[]
		)

	def gen_def_line(self, symbol: SymbolDef) -> str:
		return f'{self.hex.u32s(symbol.addr)} {symbol.type} {symbol.name}'

	def pack_def_line(self, symbol: SymbolDef, skip_addr: bool, skip_type: bool) -> str:
		if skip_addr and skip_type:
			return symbol.name
		if skip_type:
			return f'{self.hex.u32s(symbol.addr)} {symbol.name}'
		if skip_addr:
			return f'{symbol.type} {symbol.name}'
		return self.gen_def_line(symbol)

	def unpack_def_line(self, def_line: str, skip_addr: bool, skip_type: bool) -> str:
		self.chk.is_ok_str(def_line, CheckerMode.EXCEPTION)

		if skip_addr and skip_type:
			return def_line
		if skip_type or skip_addr:
			_, def_name = def_line.split()
			return def_name

		_, _, def_name = def_line.split()
		return def_name

	def compare(
		self,
		m_a: ModelDef,
		m_b: ModelDef,
		skip_addr: bool = False,
		skip_type: bool = False,
		skip_header: bool = False,
		only_names: bool = False
	) -> bool:
		if not self.is_def_model(m_a):
			raise AlephParserException('Cannot parse Model A')
		if not self.is_def_model(m_b):
			raise AlephParserException('Cannot parse Model B')

		if not skip_header:
			if m_a.head.pfw != m_b.head.pfw:
				self.log.I('Header phone firmware mismatch:')
				self.log.I('')
				self.log.I(f'{m_a.head.pfw} {m_b.head.pfw}')
				self.log.I('')
			if m_a.head.ver != m_b.head.ver:
				self.log.I('Header library version mismatch:')
				self.log.I('')
				self.log.I(f'{m_a.head.ver} {m_b.head.ver}')
				self.log.I('')
			if m_a.head.cpu != m_b.head.cpu:
				self.log.I('Header CPU architecture mismatch:')
				self.log.I('')
				self.log.I(f'{m_a.head.cpu} {m_b.head.cpu}')
				self.log.I('')

		ds_a = [self.pack_def_line(d, skip_addr=skip_addr, skip_type=skip_type) for d in m_a.syms]
		ds_b = [self.pack_def_line(d, skip_addr=skip_addr, skip_type=skip_type) for d in m_b.syms]

		set_a = set(ds_a)
		unique_to_b = [s for s in ds_b if s not in set_a]

		different = []
		for line in unique_to_b:
			for d in self.syms_by_name(m_b.syms, self.unpack_def_line(line, skip_addr, skip_type)):
				different.append(d)

		if self.chk.is_ok_list(different):
			self.log.I('Definitions model mismatch:')
			self.log.I('')
			for d in different:
				self.log.I(d.name if only_names else self.gen_def_line(d))
			self.log.I('')
			self.log.I(f'Total: {len(different)}')
			return True

		self.log.I('No model mismatch detected.')
		return False
